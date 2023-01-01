from flask import Flask
 
app = Flask(__name__)
 
@app.route("/")
def index():
    return '''<!DOCTYPE html>
<html>
<head>
         <title>Сетевое программирование - Асинхронность</title>
</head>
<body>
<h1>Сетевое программирование - Асинхронность</h1>
</body>
</html>'''
 
 
@app.route("/about")
def about():
    return "<h1>Глава 11</h1>"
 
if __name__ == "__main__":
    app.run(debug=True)
    
