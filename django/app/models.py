from django.db import models
from datetime import datetime

# Create your models here.

class REQ(models.Model):
	NOW = models.IntegerField(default=0)

class temp_module(models.Model):
	module_id = models.IntegerField(default=0)
	ip = models.CharField(max_length=50, default='0')
	temperature1 = models.FloatField(default=0.0)
	temperature2 = models.FloatField(default=0.0)
	humidity = models.FloatField(default=0.0)
	err = models.IntegerField(default=0)

class normal_module(models.Model):
	module_id = models.IntegerField(default=0)
	ip = models.CharField(max_length=50, default='0')
	temperature = models.FloatField(default=0.0)
	humidity1 = models.FloatField(default=0.0)
	humidity2 = models.FloatField(default=0.0)
	err = models.IntegerField(default=0)

class general_module(models.Model):
	Now = models.IntegerField(default=0)
	module_type = models.IntegerField(default=0)
	module_id = models.IntegerField(default=0)
	ip = models.CharField(max_length=50, default='0')
	temperature = models.FloatField(default=0.0)
	humidity = models.FloatField(default=0.0)
	val = models.FloatField(default=0.0)
	err = models.IntegerField(default=0)
	pub_date = models.DateTimeField(default=datetime.now())
	time_on = models.IntegerField(default=0)
	time_on2 = models.IntegerField(default=0)
	hours = models.IntegerField(default=0)
	minutes = models.IntegerField(default=0)
	seconds = models.IntegerField(default=0)
	hours2 = models.IntegerField(default=0)
	minutes2 = models.IntegerField(default=0)
	seconds2 = models.IntegerField(default=0)
	count = models.IntegerField(default=0)

class settingS(models.Model):
	time_on = models.IntegerField(default=100)
	module_id = models.IntegerField(default=0)
	active = models.IntegerField(default=1)
	thresh = models.FloatField(default=0.0)
	temp_high = models.FloatField(default=0.0)
	temp_high2 = models.FloatField(default=0.0)
	temp_low = models.FloatField(default=0.0)
	temp_low2 = models.FloatField(default=0.0)
	night_diff = models.FloatField(default=0.0)
	high_hour = models.IntegerField(default=0)
	low_hour = models.IntegerField(default=0)
	critical_humidity = models.IntegerField(default=560)

class general_module_plus(models.Model):
	module = models.ForeignKey(general_module)
	crit = models.IntegerField()
	crit1 = models.IntegerField()
