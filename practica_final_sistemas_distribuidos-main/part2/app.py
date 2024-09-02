from flask import Flask
from datetime import datetime

# ******************** WEB SERVICE *********************
app = Flask(__name__)

@app.route('/date-time')
def get_date_time():
    current_date_time = datetime.now().strftime("%d/%m/%Y %H:%M:%S")
    return current_date_time