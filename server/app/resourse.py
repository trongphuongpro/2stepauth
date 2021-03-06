from flask_restful import Resource, reqparse
import hashlib
from .models import User
from . import database
from . import routes
import paho.mqtt.client as paho
import numpy as np

# initialize database and tables
database.create_all()
currentUser = None


def on_connect_callback(client, userdata, flags, rc):
    print("CONNACK received with code {}.".format(rc))

# initialize MQTT client
client = paho.Client("processing_center")

client.on_connect = on_connect_callback

client.connect("test.mosquitto.org", 1883)
client.loop_start()


class UserInfoAPI(Resource):
    def __init__(self):
        super().__init__()
        self.parser = reqparse.RequestParser()
        self.parser.add_argument('username', type=str, required=True)


    def get(self, userid=None):
        if userid != None:
            return {userid: repr(User.query.filter_by(userid=userid).first())}

        return repr(User.query.all())


    def post(self):
        args = self.parser.parse_args()
        
        username = args['username']
        uid = UserCheckingAPI.create_userid(username)

        newuser = User(username=username, userid=uid)
        database.session.add(newuser)
        database.session.commit()

        return {uid: username}


    def delete(self, userid):
        pass
    

class UserCardIdAPI(Resource):
    def __init__(self):
        super().__init__()
        self.parser = reqparse.RequestParser()
        self.parser.add_argument('cardid', type=int, required=True)


    def put(self, username=None):
        global currentUser

        if username is None:
            currentUser = None
            return True
        else:
            currentUser = User.query.filter_by(username=username).first()
            print(f"currentUser: {currentUser}")

            if currentUser:
                return True
            else:
                return False


    def post(self):
        global currentUser
        print(f"currentUser: {currentUser}")

        if currentUser:
            try:
                args = self.parser.parse_args()

                currentUser.cardid = args['cardid']
                database.session.add(currentUser)
                database.session.commit()
                currentUser = None
                return 1

            except Exception as e:
                print(f"Error: {e}")

                currentUser = None
                return 0
        else:
            try:
                args = self.parser.parse_args()

                result = User.query.filter_by(cardid=args['cardid']).first()

                if result is None:
                    return 0
                else:
                    pwd = np.random.randint(1000, 9999)
                    client.publish("onetimepwd", str(pwd), qos=2)
                    return pwd
            
            except Exception as e:
                print(e)
                return 0


class UserCheckingAPI(Resource):
    def __init__(self):
        super().__init__()
        self.parser = reqparse.RequestParser()
        self.parser.add_argument('cardid', type=int, required=True)


    def get(self, username):
        uid = self.create_userid(username)

        result = User.query.filter_by(userid=uid).first()

        if result is None:
            return True

        return False


    def put(self):
        pass
        

    @classmethod
    def create_userid(self, username):
        h = hashlib.sha1()
        h.update(username.encode('utf-8'))

        return h.hexdigest()


def create_api():
    routes.add_resource(UserInfoAPI, '/api/user/<string:userid>', '/api/user')
    routes.add_resource(UserCardIdAPI, '/api/uid/<string:username>', '/api/uid')
    routes.add_resource(UserCheckingAPI, '/api/checking/<string:username>')