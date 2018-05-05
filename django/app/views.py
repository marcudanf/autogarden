from django.shortcuts import render, get_object_or_404
from django.http import HttpResponse, Http404
from django.core.mail import send_mail
from .models import normal_module, temp_module, general_module, settingS, general_module_plus
from datetime import datetime, timedelta, timezone, tzinfo
from django.db import models
from .forms import FORM #, settings_form
import urllib.request

mail_addr = [
	'rasadnic@248chilli.ro',
#	'marcudanf@gmail.com',
#	'predrag.jivici@studiostatic.com',
#	'zoranbug@yahoo.com'
]

addr = 'rasadnic@248chilli.ro'

def index(request):
#	print('running rasadnic default')
	return render(request, 'index.html', {})


def rasadnic(request):
#	print('running rasadnic rasadnic')
#	send_mail('test','un mail de test',addr,mail_addr)
#	Settings = settingS(time_on=100,active=1,module_id=100,thresh=26.0,temp_high=31,temp_high2=30,temp_low=22,temp_low2=23,night_diff=12,high_hour=18,low_hour=8,critical_humidity=610)
#	Settings.save()
	all_entries=[]
	for x in range(35):
		try:
			curr_time = datetime.now(timezone.utc)
			prev_time = general_module.objects.filter(module_id=x).latest('pub_date').pub_date
			time_diff = (curr_time - prev_time) / timedelta(hours=1)
			if time_diff <= 2:
				all_entries.append(general_module.objects.filter(module_id=x).latest('pub_date'))
				E=all_entries[-1]
				if E.module_type == 0:
					E.humidity = int(E.humidity)
					E.val = int(E.val)
		except general_module.DoesNotExist:
			pass
	all_entries_plus=[]
	for entry in all_entries:
		if entry.module_type == 0:
			Sett = settingS.objects.filter(module_id=entry.module_id).last()
			all_entries_plus.append(general_module_plus(module=entry,crit1=(Sett.critical_humidity+40), crit=Sett.critical_humidity))
		else:
			all_entries_plus.append(general_module_plus(module=entry,crit1=0, crit=0))
	return render(request, 'rasadnic.html', {'entries' : all_entries_plus, 'Now' : datetime.now()})

def settings(request):
#	print('running rasadnic setting')
	Settings = settingS.objects.filter(module_id=100).last()
	if request.method == "POST":
		ip0=[]
		ip1=[]
		for x in range(50):
			try:
				E = general_module.objects.filter(module_id=x).latest('pub_date')
				if E.ip == '0.0.0.0':
					X = general_module.objects.filter(module_id=x)
					i = 0
					for A in X:
						i = i + 1
					while i >= 0:
						i = i - 1
						if X[i].ip != '0.0.0.0':
							if E.module_type == 0:
								ip0.append(X[i].ip)
							elif E.module_type == 1:
								ip1.append(X[i].ip)
							break
				else:
					if E.module_type == 0:
						ip0.append(E.ip)
					elif E.module_type == 1:
						ip1.append(E.ip)
			except general_module.DoesNotExist:
				pass
		for ip in ip0:
			print(ip)
		for ip in ip1:
			print(ip)
		Settings = settingS.objects.filter(module_id=100).last()
		if request.POST.get('time_on', '') != '':
			Settings.time_on = request.POST.get('time_on')
			for ip in ip0:
				S_id = general_module.objects.filter(ip=ip).last().module_id
				SS = settingS.objects.filter(module_id=S_id).last().active
				if SS == 0:
					url='http://' + ip + '/liters=' + str(Settings.time_on * 0.05)
					try:
						urllib.request.urlopen(url)
					except urllib.error.URLError as e:
						pass
		if request.POST.get('critical_humidity', '') != '':
			Settings.critical_humidity = request.POST.get('critical_humidity')
			for ip in ip0:
				S_id = general_module.objects.filter(ip=ip).last().module_id
				SS = settingS.objects.filter(module_id=S_id).last().active
				if SS == 0:
					url='http://' + ip + '/humidity_threshold=' + str(Settings.critical_humidity)
					try:
						urllib.request.urlopen(url)
					except urllib.error.URLError as e:
						pass
		if request.POST.get('temp_high', '') != '':
			Settings.temp_high = request.POST.get('temp_high')
			for ip in ip1:
				url='http://' + ip + '/t_h=' + str(Settings.temp_high)
				try:
					urllib.request.urlopen(url)
				except urllib.error.URLError as e:
					pass
		if request.POST.get('temp_high2', '') != '':
			Settings.temp_high2 = request.POST.get('temp_high2')
			for ip in ip1:
				url='http://' + ip + '/t_h2=' + str(Settings.temp_high2)
				try:
					urllib.request.urlopen(url)
				except urllib.error.URLError as e:
					pass
		if request.POST.get('temp_low','') != '':
			Settings.temp_low = request.POST.get('temp_low')
			for ip in ip1:
				url='http://' + ip + '/t_l=' + str(Settings.temp_low)
				try:
					urllib.request.urlopen(url)
				except urllib.error.URLError as e:
					pass
		if request.POST.get('temp_low2','') != '':
			Settings.temp_low2 = request.POST.get('temp_low2')
			for ip in ip1:
				url='http://' + ip + '/t_l2=' + str(Settings.temp_low2)
				try:
					urllib.request.urlopen(url)
				except urllib.error.URLError as e:
					pass
		if request.POST.get('night_diff','') != '':
			Settings.night_diff = request.POST.get('night_diff')
			for ip in ip1:
				url='http://' + ip + '/night_thresh=' + str(Settings.night_diff)
				try:
					urllib.request.urlopen(url)
				except urllib.error.URLError as e:
					pass
		if request.POST.get('high_hour','') != '':
			Settings.high_hour = request.POST.get('high_hour')
			for ip in ip1:
				url='http://' + ip + '/high_hour=' + str(Settings.high_hour)
				try:
					urllib.request.urlopen(url)
				except urllib.error.URLError as e:
					pass
		if request.POST.get('low_hour','') != '':
			Settings.low_hour = request.POST.get('low_hour')
			for ip in ip1:
				url='http://' + ip + '/low_hour=' + str(Settings.low_hour)
				try:
					urllib.request.urlopen(url)
				except urllib.error.URLError as e:
					pass
		if request.POST.get('thresh','') != '':
			Settings.thresh = request.POST.get('thresh')
			for ip in ip0:
				S_id = general_module.objects.filter(ip=ip).last().module_id
				SS = settingS.objects.filter(module_id=S_id).last().active
				if SS == 0:
					url='http://' + ip + '/temperature_threshold=' + str(float(Settings.thresh) - 2)
					try:
						urllib.request.urlopen(url)
					except urllib.error.URLError as e:
						pass
		if request.POST.get('critical_humidity','') != '':
#			print("found")
			Settings.critical_humidity = request.POST.get('critical_humidity')
		Settings.save()
	Settings = settingS.objects.filter(module_id=100).last()
	return render(request,'settings.html', {'settings' : Settings})

def upload(request):
#	print('running rasadnic upload')
	if request.method == "GET":
		Type = request.GET.get('type')
		r_id = request.GET.get('id')
		if Type == 'normal_module':
#			print("normal")
			Settings = settingS.objects.filter(module_id=r_id).last()
			if Settings == None:
				SETTINGS = settingS.objects.filter(module_id=100).last()
				Settings = settingS(time_on=SETTINGS.time_on,active=0,module_id=r_id,thresh=SETTINGS.thresh,temp_high=SETTINGS.temp_high,temp_high2=SETTINGS.temp_high2,temp_low=SETTINGS.temp_low,temp_low2=SETTINGS.temp_low2,night_diff=SETTINGS.night_diff,critical_humidity=SETTINGS.critical_humidity,high_hour=SETTINGS.high_hour,low_hour=SETTINGS.low_hour)
				Settings.save()
			Module_type = 0
			Module_id = request.GET.get('id')
			Module_ip = request.GET.get('ip')
			Module_temperature = request.GET.get('temperature')
			Module_humidity1 = request.GET.get('humidity1')
			Module_humidity2 = request.GET.get('humidity2')
			Module_err = request.GET.get('err')
			Module_time_on = request.GET.get('time_on')
			Module_hours = int(int(Module_time_on)/3600)
			Module_minutes = int((int(Module_time_on)%3600)/60)
			Module_seconds = int(Module_time_on)%60
			Module_now = 0
			if request.GET.get('now', '') != '':
				Module_now = request.GET.get('now')
			Module = general_module(Now=Module_now, module_type=Module_type, module_id=Module_id, ip=Module_ip, temperature=Module_temperature, humidity=Module_humidity1, val=Module_humidity2, err=Module_err, pub_date=datetime.now(), time_on=Module_time_on, hours=Module_hours, minutes=Module_minutes, seconds=Module_seconds)
			Module.save()
			if int(Module_humidity1) < Settings.critical_humidity or int(Module_humidity2) < Settings.critical_humidity:
				subject = 'Tava '+ str(Module_id) + ' trebuie udata'
				message = 'link spre site:http://chilli.go.ro/rasadnic/modul/' + str(Module_id)
				send_mail(subject,message,addr,mail_addr,fail_silently=False)
			if float(Module_temperature) <10:
				subject = 'Probleme la tava ' + str(Module_id)
				message = 'Probleme cu incalzirea http://chilli.go.ro/rasadnic/modul/' + str(Module_id)
				send_mail(subject,message,addr,mail_addr,fail_silently=False)
			if int(Module_err) == 2:
				subject = 'Erorre la tava ' + str(Module_id)
				message = 'Senzorul de temperatura are probleme http://chilli.go.ro/rasadnic/modul/' + str(Module_id)
				send_mail(subject,message,addr,mail_addr,fail_silently=False)
			return HttpResponse("hour="+str(datetime.now().hour)+'!')
		elif Type == 'temp_module':
#			print("temp")
			Settings = settingS.objects.filter(module_id=r_id).last()
			if Settings == None:
				SETTINGS = settingS.objects.filter(module_id=100).last()
				Settings = settingS(time_on=SETTINGS.time_on,active=0,module_id=r_id,thresh=SETTINGS.thresh,temp_high=SETTINGS.temp_high,temp_high2=SETTINGS.temp_high2,temp_low=SETTINGS.temp_low,temp_low2=SETTINGS.temp_low2,night_diff=SETTINGS.night_diff,critical_humidity=SETTINGS.critical_humidity,high_hour=SETTINGS.high_hour,low_hour=SETTINGS.low_hour)
				Settings.save()
			Settings.temp_high = request.GET.get('temperature_high')
			Settings.temp_low = request.GET.get('temperature_low')
			Settings.night_diff = request.GET.get('night_diff')
			Settings.high_hour = request.GET.get('high_hour')
			Settings.low_hour = request.GET.get('low_hour')
			Settings.save()
			Module_type = 1
			Module_id = request.GET.get('id')
			Module_ip = request.GET.get('ip')
			Module_temperature1 = request.GET.get('temperature1')
			Module_temperature2 = request.GET.get('temperature2')
			Module_humidity = request.GET.get('humidity')
			Module_err = request.GET.get('err')
			Module_time_on = request.GET.get('time_on')
			Module_time_on2 = request.GET.get('time_on2')
			Module_hours = int(int(Module_time_on)/3600)
			Module_minutes = int((int(Module_time_on)%3600)/60)
			Module_seconds = int(Module_time_on)%60
			Module_hours2 = int(int(Module_time_on2)/3600)
			Module_minutes2 = int((int(Module_time_on2)%3600)/60)
			Module_seconds2 = int(Module_time_on2)%60
			Module_count = request.GET.get('count')
			Module_now = 0
			Module = general_module(Now=Module_now, module_type=Module_type, module_id=Module_id, ip=Module_ip, temperature=Module_temperature1, val=Module_temperature2, humidity=Module_humidity ,err=Module_err, pub_date=datetime.now(), time_on=Module_time_on, time_on2=Module_time_on2, hours=Module_hours, minutes=Module_minutes, seconds=Module_seconds, hours2=Module_hours2, minutes2=Module_minutes2, seconds2=Module_minutes2, count=Module_count)
			Module.save()
			if float(Module_temperature1) < 5 or float(Module_temperature1) > 40: #10 initial
				subject = 'Probleme cu controlul aerului'
				message = 'Probleme la controlul aerului http://chilli.go.ro/rasadnic/modul/' + str(Module_id)
				send_mail(subject,message,addr,mail_addr,fail_silently=False)
			if int(Module_err) != 0:
				subject = 'Eroare la senzorul de temperatura al aerului'
				message = 'Senzorul de temperatura a al aerului nu mai merge: http://chilli.go.ro/rasadnic/modul/' + str(Module_id)
				send_mail(subject,message,addr,mail_addr,fail_silently=False)
			return HttpResponse("hour="+str(datetime.now().hour)+'!')
		else:
			return render(request, '502.html', {})
	else:
		return render(request, '502.html', {})

def module(request,Number):
#	print('running rasadnic module ' + str(Number))
	err = 0
	try:
		entry = general_module.objects.filter(module_id=Number).latest('pub_date')
	except general_module.DoesNotExist:
		err = 1
		return render(request, '404.html', {})
	if err == 0:
		if entry.module_type == 0:
			entry.humidity = int(entry.humidity)
			entry.val = int(entry.val)
			return render(request, 'module.html', {'entry' : entry})
		else:
			return render(request, 'air.html', {'entry' : entry})

def details(request,Number):
#	print('running rasadnic details ' + str(Number))
	err = 0
	try:
		entry = general_module.objects.filter(module_id=Number).latest('pub_date')
	except general_module.DoesNotExist:
		err = 1
		return render(request, '404.html', {})
	if err == 0:
		not_ordered = general_module.objects.filter(module_id=Number)
		all_entries = not_ordered.order_by('-pub_date')
		E = all_entries[:300]
		if entry.module_type == 0:
			for p in E:
				p.humidity = int(p.humidity)
				p.val = int(p.val)
			return render(request, 'log.html', {'entry' : E, 'Number' : Number})
		else:
			return render(request, 'air_log.html', {'entry' : E, 'Number' : Number})

def settingsNo(request, Number):
#	print('running rasadnic settings ' + str(Number))
	try:
		Settings = settingS.objects.filter(module_id=Number).last()
		if Settings == None:
#			print('not found')
			SETTINGS = settingS.objects.filter(module_id=100).last()
			Settings = settingS(time_on=SETTINGS.time_on,active=0,module_id=Number,thresh=SETTINGS.thresh,temp_high=SETTINGS.temp_high,temp_high2=SETTINGS.temp_high2,temp_low=SETTINGS.temp_low,temp_low2=SETTINGS.temp_low2,night_diff=SETTINGS.night_diff,critical_humidity=SETTINGS.critical_humidity,high_hour=SETTINGS.high_hour,low_hour=SETTINGS.low_hour)
			Settings.save()
	except settingS.DoesNotExist:
#		print('ran exception')
		SETTINGS = settingS.objects.filter(module_id=100).last()
		Settings = settingS(time_on=SETTINGS.time_on,active=0,module_id=Number,thresh=SETTINGS.thresh,temp_high=SETTINGS.temp_high,temp_high2=SETTINGS.temp_high2,temp_low=SETTINGS.temp_low,temp_low2=SETTINGS.temp_low2,night_diff=SETTINGS.night_diff,critical_humidity=SETTINGS.critical_humidity,high_hour=SETTINGS.high_hour,low_hour=SETTINGS.low_hour)
		Settings.save()
	try:
		Settings = settingS.objects.filter(module_id=Number).last()
		SETTINGS = settingS.objects.filter(module_id=100).last()
		err = 0
		if request.method == 'POST':
			if request.POST.get('active', '') != '':
				Settings.active = 1
			else:
				Settings.active = 0
			Settings.save()
			ip = general_module.objects.filter(module_id=Number).latest('pub_date').ip
			if ip == '0.0.0.0':
				X = general_module.objects.filter(module_id=Number)
				i = 0
				for A in X:
					i = i + 1
				while i >= 0:
					i = i - 1
					if X[i].ip != '0.0.0.0':
						ip = X[i].ip
			if general_module.objects.filter(module_id=Number).latest('pub_date').module_type == 0:
				url = ''
				if request.POST.get('critical_humidity', '') != '':
#					print(request.POST.get('critical_humidity'))
					Settings.critical_humidity = request.POST.get('critical_humidity')
					Settings.save()
				if Settings.active == 1:
					url = 'http://' + ip + '/humidity_threshold=' + str(Settings.critical_humidity)
				else:
					url = 'http://' + ip + '/humidity_threshold=' + str(SETTINGS.critical_humidity)
				if url != '':
#					print(url)
					try:
						urllib.request.urlopen(url)
					except urllib.error.URLError as e:
						pass
				url = ''
				if request.POST.get('thresh', '') != '':
					Settings.thresh = request.POST.get('thresh')
					Settings.save()
				if Settings.active == 1:
					url = 'http://' + ip + '/temperature_threshold=' + str(float(Settings.thresh)-2)
				else:
					url = 'http://' + ip + '/temperature_threshold=' + str(float(SETTINGS.thresh)-2)
				if url != '':
#					print(url)
					try:
						urllib.request.urlopen(url)
					except urllib.error.URLError as e:
						pass
				url = ''
				if request.POST.get('time_on', '') != '':
#					print(request.POST.get('time_on'))
					Settings.time_on = request.POST.get('time_on')
					Settings.save()
				if Settings.active == 1:
					url = 'http://' + ip + '/liters=' + str(Settings.time_on * 0.05)
				else:
					url = 'http://' + ip + '/liters=' + str(SETTINGS.time_on * 0.05)
				if url != '':
#					print(url)
					try:
						urllib.request.urlopen(url)
					except urllib.error.URLError as e:
						pass
			Settings.save()
		try:
			entry = general_module.objects.filter(module_id=Number).latest('pub_date')
		except general_module.DoesNotExist:
			err = 1
			return render(request, '404.html', {})
		if err == 0:
			if entry.module_type == 0:
				return render(request, 'settings0.html', {'settings' : Settings, 'Number' : Number})
			else:
				return render(request, 'settings1.html', {'settings' : Settings, 'Number' : Number})
	except settingS.DoesNotExist:
		return render(request, '502.html', {})

def restart(request, Number):
	try:
		ip = general_module.objects.filter(module_id=Number).latest('pub_date').ip
	except general_module.DoesNotExist:
		return render(request, '404.html', {})
	url = 'http://' + ip + '/restart'
	if ip == '0.0.0.0':
		X = general_module.objects.filter(module_id=Number)
		i = 0
		for A in X:
			i = i + 1
			while i >= 0:
				i = i - 1
				if X[i].ip != '0.0.0.0':
					ip = X[i].ip
	try:
		urllib.request.urlopen(url)
	except urllib.error.URLError as e:
		pass
	return HttpResponse('<center><br/><br/><h1>Have a little bit of patiente,<br/>you are being redirected<br/>with the <a target="_blank" href="https://en.wikipedia.org/wiki/Speed_of_light">speed of light</a>!<br/>(takes a while)</h1></center><meta http-equiv="refresh" content="0; URL=\'http://electro.go.ro/rasadnic\'" />')
