from flask_restful import Resource, reqparse
import hashlib
from .models import User
from . import database
from . import routes

# initialize database and tables
database.create_all()
currentUser = None

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


    def put(self, username):
        global currentUser

        currentUser = User.query.filter_by(username=username).first()
        print(f"currentUser: {currentUser}")
        if currentUser:
            return {"status": True}
        else:
            return {"status": False}


    def post(self):
        global currentUser
        print(f"currentUser: {currentUser}")

        if currentUser:
            args = self.parser.parse_args()

            currentUser.cardid = args['cardid']
            database.session.add(currentUser)
            database.session.commit()
            currentUser = None

            return {"status": True}

        return {"status": False}


class UserCheckingAPI(Resource):
    def get(self, username):
        uid = self.create_userid(username)

        result = User.query.filter_by(userid=uid).first()

        if result is None:
            return {"status": True}

        return {"status": False}


    @classmethod
    def create_userid(self, username):
        h = hashlib.sha1()
        h.update(username.encode('utf-8'))

        return h.hexdigest()


def create_api():
    routes.add_resource(UserInfoAPI, '/api/user/<string:userid>', '/api/user')
    routes.add_resource(UserCardIdAPI, '/api/uid/<string:username>', '/api/uid')
    routes.add_resource(UserCheckingAPI, '/api/checking/<string:username>')