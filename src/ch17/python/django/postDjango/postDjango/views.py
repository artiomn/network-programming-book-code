from django.http import HttpResponse
from django.shortcuts import render


def index(request):
      return render(request, 'index.html')


def validate(request):
   if request.method == 'POST':
      username= request.POST["user"]
      password = request.POST["pass"]
      dict = {
         'username': username,
         'password': password
      }
      return render(request, 'validate.html', dict)
