from app import application
from app.api import create_api

create_api()


if __name__ == '__main__':
    application.run(host="localhost", debug=False)