from app import database


class User(database.Model):
    userid = database.Column(database.String(40), unique=True, nullable=False, primary_key=True)
    username = database.Column(database.String(20), unique=True, nullable=False)
    cardid = database.Column(database.Integer, unique=True, nullable=True)
    

    def __repr__(self):
        return f'<User {self.username!r}>'