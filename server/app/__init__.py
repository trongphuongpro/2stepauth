from flask import Flask
from flask_restful import Api
from flask_sqlalchemy import SQLAlchemy

db_path = '/home/trongphuong/Work/2stepauth/server/userinfo.db'

application = Flask(__name__)
application.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///' + db_path

database = SQLAlchemy(application)
routes = Api(application)