from django.shortcuts import render
from django.core.mail import send_mail

def index(request):
#	print("running default index")
	return render(request, 'index.html', {})

