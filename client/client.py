from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
from PyQt5.QtGui import *
from requests import get, put, post
import sys
import json


class RegisterDialog(QDialog):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.userinfo = {'username': None, 'userid': None}
        self.validUsername = True

        self.setWindowTitle("Register new card")

        buttons = QDialogButtonBox.Ok | QDialogButtonBox.Cancel

        buttonBox = QDialogButtonBox(buttons)
        buttonBox.accepted.connect(self.sendRegisterRequest)
        buttonBox.rejected.connect(self.reject)

        self.status_text = QLabel()

        username_field = QLineEdit()
        username_field.setMaxLength(20)
        username_field.setPlaceholderText("Enter your username")
        username_field.textChanged.connect(self.getUsername)

        layout = QVBoxLayout()
        layout.addWidget(username_field)
        layout.addWidget(self.status_text)
        layout.addWidget(buttonBox)

        self.setLayout(layout)


    def getUsername(self, text):
        self.userinfo['username'] = text
        self.checkUsername(text)


    def sendRegisterRequest(self):
        if self.validUsername:
            try:
                res = post("http://localhost:5000/api/user", data={"username":self.userinfo["username"]}).json()
                self.userinfo["userinfo"] = list(res.keys())[0]
                print(f"[userid] {res}")

            except:
                self.status_text.setText("Disconnected to server")
            else:
                self.accept()
        else:
            self.status_text.setText("Invalid username")


    # check if username is valid
    def checkUsername(self, username):
        if username:
            try:
                res = get(f"http://localhost:5000/api/checking/{username}").json()
                if (res["status"] is True):
                    self.validUsername = True
                    self.status_text.setText("You can use this username")
                else:
                    self.validUsername = False
                    self.status_text.setText("This username is existed")
            except:
                self.status_text.setText("Disconnected to server")
                
        else:
            self.validUsername = False


class ClientApp(QMainWindow):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.setWindowTitle("Client")

        text = QLabel()
        text.setAlignment(Qt.AlignCenter)
        text.setText("ONE-TIME PASSWORD")

        password = QLabel()
        password.setAlignment(Qt.AlignCenter)
        font = password.font()
        font.setPointSize(20)
        password.setFont(font)
        password.setText("1234")

        register_button = QPushButton("Register new card")
        register_button.clicked.connect(self.register_button_callback)

        #statusbar = QStatusBar()

        layout = QVBoxLayout()
        layout.addWidget(text)
        layout.addWidget(password)
        layout.addWidget(register_button)

        widget = QWidget()
        widget.setLayout(layout)

        #self.setStatusBar(statusbar)
        self.setCentralWidget(widget)


    def register_button_callback(self):
        register_dialog = RegisterDialog(self)
        register_dialog.exec()


app = QApplication(sys.argv)
clientapp = ClientApp()
clientapp.show()
app.exec()