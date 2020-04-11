from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
from PyQt5.QtGui import *
from requests import get, put, post
import paho.mqtt.client as paho
import sys
import json

URL = "http://192.168.0.99:5000"

def on_message_callback(client, userdata, msg):
    clientapp.password.setText(msg.payload.decode())

def on_subscribe_callback(client, userdata, mid, granted_qos):
    print("Subscribed: "+str(mid)+" "+str(granted_qos))

def on_connect_callback(client, userdata, flags, rc):
    print("CONNACK received with code {}.".format(rc))

# initialize MQTT client
client = paho.Client("client_app")

client.on_message = on_message_callback
client.on_subscribe = on_subscribe_callback
client.on_connect = on_connect_callback

client.connect("test.mosquitto.org", 1883)
client.subscribe("onetimepwd", 2)
client.loop_start()


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
                res = post(f"{URL}/api/user", data={"username":self.userinfo["username"]}).json()
                self.userinfo["userinfo"] = list(res.keys())[0]

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
                res = get(f"{URL}/api/checking/{username}").json()
                if (res is True):
                    self.validUsername = True
                    self.status_text.setText("You can use this username")
                else:
                    self.validUsername = False
                    self.status_text.setText("This username is existed")
            except:
                self.status_text.setText("Disconnected to server")
                
        else:
            self.validUsername = False


class ActivationDialog(QDialog):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.userinfo = {'username': None, 'userid': None}

        self.setWindowTitle("Activate new card")

        buttons = QDialogButtonBox.Ok | QDialogButtonBox.Cancel

        buttonBox = QDialogButtonBox(buttons)
        buttonBox.accepted.connect(self.sendActivationRequest)
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


    def sendActivationRequest(self):
        try:
            res = put(f"{URL}/api/uid/{self.userinfo['username']}").json()

            if res is False:
                messagebox = QMessageBox(self)
                messagebox.setText("Invalid username")
                messagebox.exec()

        except:
            self.status_text.setText("Disconnected to server")
        else:
            self.accept()


class ClientApp(QMainWindow):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.setWindowTitle("Client")

        text = QLabel()
        text.setAlignment(Qt.AlignCenter)
        text.setText("ONE-TIME PASSWORD")

        self.password = QLabel()
        self.password.setAlignment(Qt.AlignCenter)
        font = self.password.font()
        font.setPointSize(20)
        self.password.setFont(font)
        self.password.setText("____")

        register_button = QPushButton("Register new card")
        register_button.clicked.connect(self.register_button_callback)

        activation_button = QPushButton("Activate new card")
        activation_button.clicked.connect(self.activation_button_callback)


        #statusbar = QStatusBar()

        layout = QVBoxLayout()
        layout.addWidget(text)
        layout.addWidget(self.password)
        layout.addWidget(register_button)
        layout.addWidget(activation_button)

        widget = QWidget()
        widget.setLayout(layout)

        #self.setStatusBar(statusbar)
        self.setCentralWidget(widget)


    def register_button_callback(self):
        register_dialog = RegisterDialog(self)
        register_dialog.exec()


    def activation_button_callback(self):
        activation_dialog = ActivationDialog(self)
        activation_dialog.exec()



app = QApplication(sys.argv)
clientapp = ClientApp()
clientapp.show()
app.exec()