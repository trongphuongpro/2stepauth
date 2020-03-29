from flask import Flask
from flask_restful import Api
from flask_sqlalchemy import SQLAlchemy

application = Flask(__name__)
application.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///' + '/home/trongphuong/Work/RESTserver/userinfo.db'

database = SQLAlchemy(application)
routes = Api(application)