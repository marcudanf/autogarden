from django import forms
from django.forms import ModelForm
from .models import settingS

#class normal_form(forms.ModelForm):
#	class Meta:
#		model = normal_module
#		fields = [
#			'module_id',
#			'ip',
#			'temperature',
#			'humidity1',
#			'humidity2',
#			'err',
#		]
#class temp_form(forms.ModelForm):
#	class Meta:
#		model = temp_module
#		fields = [
#			'module_id',
#			'ip',
#			'humidity',
#			'temperature1',
#			'temperature2',
#			'err',
#		]

class high_temp_form(forms.Form):
	fields = [
		'high_temp'
	]
class low_temp_form(forms.Form):
	fields = [
		'low_temp'
	]
class night_diff_form(forms.Form):
	fields = [
		'night_diff'
	]
class high_hour(forms.Form):
	fields = [
		'high_hour'
	]
class low_hour(forms.Form):
	fields = [
		'low_hour'
	]
class thresh(forms.Form):
	fields = [
		'thresh'
	]
class crit_humidity(forms.Form):
	fields = [
		'crit_humidity'
	]
#class settings_form(forms.ModelForm):
#	class Meta:
#		model = settings
class FORM(forms.ModelForm):
	temp_high = forms.FloatField(required=False)
	temp_low = forms.FloatField(required=False)
	night_diff = forms.FloatField(required=False)
	high_hour = forms.IntegerField(required=False)
	low_hour = forms.IntegerField(required=False)
	threshold = forms.FloatField(required=False)
	critical_humidity = forms.IntegerField(required=False)
	class Meta:
		model = settingS
		fields = ['temp_high','temp_low','night_diff','high_hour','low_hour','threshold','critical_humidity']
