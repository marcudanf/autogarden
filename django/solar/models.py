from django.db import models
from datetime import datetime

# Create your models here.

class Solar(models.Model):
	pub_date = models.DateTimeField(default=datetime.now())
	ip = models.CharField(max_length=50, default='0')
	no = models.IntegerField(default=0)
	active = models.IntegerField(default=0)
	on1 = models.IntegerField(default=0)
	on2 = models.IntegerField(default=0)
	time1 = models.IntegerField(default=0)
	time2 = models.IntegerField(default=0)
	dur1 = models.IntegerField(default=0)
	dur2 = models.IntegerField(default=0)
	ok1 = models.IntegerField(default=0)
	ok2 = models.IntegerField(default=0)
	days = models.IntegerField(default=0)
