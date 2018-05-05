from django.shortcuts import render
from django.http import HttpResponse
from .models import Solar
from django.core.mail import send_mail
from django.db import models
import urllib.request
from datetime import datetime, timedelta, timezone, tzinfo

# Create your views here.

def solar(request):
	all_entries = []
	for x in range(15):
		try:
			all = Solar.objects.filter(no=x)
			if len(all):
				curr_time = datetime.now(timezone.utc)
				prev_time=all.last().pub_date
				time_diff = (curr_time - prev_time) / timedelta(hours=1)
				all_entries.append(all.last())
#				print(time_diff)
				if time_diff > 1.3 and all_entries[-1].active == 1:
					all_entries[-1].active = -1
					all_entries[-1].ok1 = 0
					all_entries[-1].ok2 = 0
		except Solar.DoesNotExist:
			pass
#	for x in all_entries:
#		print(x.active)
	return render(request, 'solar.html', {'entries' : all_entries})

def upload(request):
	if request.method == 'GET':
		Module_ip = request.GET.get('ip')
		Module_no = int(request.GET.get('no'))
		Module_ok1 = int(request.GET.get('ok1'))
		Module_ok2 = int(request.GET.get('ok2'))
		try:
			solar = Solar.objects.filter(no=Module_no).last()
			if solar == None:
				solar = Solar(pub_date=datetime.now(),ip=Module_ip,no=Module_no,active=1,on1=1,on2=1,time1=8,time2=21,dur1=20,dur2=20,ok1=0,ok2=0,days=1)
		except Solar.DoesNotExist:
			solar = Solar(pub_date=datetime.now(),ip=Module_ip,no=Module_no,active=1,on1=1,on2=1,time1=8,time2=21,dur1=20,dur2=20,ok1=0,ok2=0,days=1)
		if Module_ip == '0.0.0.0':
			Module_ip = solar.ip
		solar.pub_date = datetime.now()
		solar.ip = Module_ip
		solar.ok1 = Module_ok1
		solar.ok2 = Module_ok2
		solar.save()
	return HttpResponse('h='+str(datetime.now().hour)+'!')

def solarx(request, Number):
	try:
		solar = Solar.objects.filter(no=Number).last()
		if solar == None:
			return render(request, '404.html', {})
	except Solar.DoesNotExist:
		return render(request, '404.html', {})
	if request.method == 'POST':
		ip = solar.ip
		url = 'http://' + ip + '/?solar=' + str(Number)
		if request.POST.get('active', '') != '':
			url = url + '&active=1'
			solar.active = 1
		else:
			url = url + '&active=0'
			solar.active = 0
		if request.POST.get('on1', '') != '':
			url = url + '&on1=1'
			solar.on1 = 1
		else:
			url = url + '&on1=0'
			solar.on1 = 0
		if request.POST.get('on2', '') != '':
			url = url + '&on2=1'
			solar.on2 = 1
		else:
			url = url + '&on2=0'
			solar.on2 = 0
		if request.POST.get('time1', '') != '':
			x = request.POST.get('time1')
			url = url + '&h1=' + x
			solar.time1 = int(x)
		else:
			x = solar.time1
			url = url + '&h1=' + str(x)
		if request.POST.get('time2', '') != '':
			x = request.POST.get('time2')
			url = url + '&h2=' + x
			solar.time2 = int(x)
		else:
			x = solar.time2
			url = url + '&h2=' + str(x)
		if request.POST.get('dur1', '') != '':
			x = request.POST.get('dur1')
			url = url + '&t1=' + x
			solar.dur1 = int(x)
		else:
			x = solar.dur1
			url = url + '&t1=' + str(x)
		if request.POST.get('dur2', '') != '':
			x = request.POST.get('dur2')
			url = url + '&t2=' + x
			solar.dur2 = int(x)
		else:
			x = solar.dur2
			url = url + '&t2=' + str(x)
		if request.POST.get('days', '') != '':
			x = request.POST.get('days')
			url = url + '&day=' + x
			solar.days = int(x)
		else:
			x = solar.days
			url = url + '&day=' + str(x)
#		print(url)
		try:
			urllib.request.urlopen(url)
			solar.save()
		except urllib.error.URLError as e:
			return HttpResponse('<center><h1>ERR</h1></center>')
	return render(request, 'solarx.html', {'solar' : solar})

