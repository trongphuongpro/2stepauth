from flask_restful import Resource, reqparse
import hashlib
from app.models import User
from app import database
from app import routes


currentId = None

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
        global currentId

        args = self.parser.parse_args()
        
        uid = UserChecking.create_userid(args['username'])

        database[uid] = {'username':args['username']}
        currentId = uid

        return {uid: database[uid]}


    def delete(self, userid):
        del database[userid]

        return database
    

class UserIdAPI(Resource):
    def __init__(self):
        super().__init__()
        self.parser = reqparse.RequestParser()
        self.parser.add_argument('cardid', type=int, required=True)


    def put(self):
        args = self.parser.parse_args()
        
        database[currentId]["cardid"] = args["cardid"]

        return {currentId: database[currentId]}


class UserCheckingAPI(Resource):
    def get(self, username):
        uid = self.create_userid(username)

        if uid in database.keys():
            return {"status": False}

        return {"status": True}


    @classmethod
    def create_userid(self, username):
        h = hashlib.sha1()
        h.update(username.encode('utf-8'))

        return h.hexdigest


def create_api():
    routes.add_resource(UserInfoAPI, '/api/user/<string:userid>', '/api/user')
    routes.add_resource(UserIdAPI, '/api/uid')
    routes.add_resource(UserCheckingAPI, '/api/checking/<string:username>')