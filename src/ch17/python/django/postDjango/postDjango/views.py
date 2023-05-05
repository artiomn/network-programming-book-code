from django.shortcuts import render


def index(request):
    return render(request, 'index.html')


def validate(request):
    if 'POST' == request.method:
        ret_dict = {'username': request.POST['user'], 'password': request.POST['pass']}
        return render(request, 'validate.html', ret_dict)
    return None
