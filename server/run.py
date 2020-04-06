from app import application
from app.resourse import create_api

create_api()


if __name__ == '__main__':
    application.run(host="192.168.0.99", debug=False)