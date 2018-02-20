# -*- coding: utf-8 -*-
# Generated by Django 1.11.7 on 2017-12-30 15:45
from __future__ import unicode_literals

import datetime
from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('app', '0012_auto_20171230_1413'),
    ]

    operations = [
        migrations.AddField(
            model_name='settings',
            name='temp_high2',
            field=models.FloatField(default=0.0),
        ),
        migrations.AddField(
            model_name='settings',
            name='temp_low2',
            field=models.FloatField(default=0.0),
        ),
        migrations.AlterField(
            model_name='general_module',
            name='pub_date',
            field=models.DateTimeField(default=datetime.datetime(2017, 12, 30, 17, 45, 27, 455185)),
        ),
    ]
