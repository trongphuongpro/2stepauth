from requests import get, put, post, delete


res = post("http://localhost:5000/api/user", data={"username":"user_1"}).json()
print(f"[post] {res}")
res = put("http://localhost:5000/api/uid", data={"cardid":0x1234}).json()
print(f"[put] {res}")
print("-------------------------------")

res = post("http://localhost:5000/api/user", data={"username":"user_2"}).json()
print(f"[post] {res}")
res = put("http://localhost:5000/api/uid", data={"cardid":0xABCD}).json()
print(f"[put] {res}")
print("-------------------------------")

res = post("http://localhost:5000/api/user", data={"username":"user_3"}).json()
print(f"[post] {res}")
res = put("http://localhost:5000/api/uid", data={"cardid":0x12AB}).json()
print(f"[put] {res}")
print("-------------------------------")

res = get(f"http://localhost:5000/api/user").json()
print(f"[get] {res}")

