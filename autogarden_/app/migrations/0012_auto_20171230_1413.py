# -*- coding: utf-8 -*-
# Generated by Django 1.11.7 on 2017-12-30 12:13
from __future__ import unicode_literals

import datetime
from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('app', '0011_auto_20171230_1258'),
    ]

    operations = [
        migrations.AddField(
            model_name='general_module',
            name='minutes2',
            field=models.IntegerField(default=0),
        ),
        migrations.AlterField(
            model_name='general_module',
            name='pub_date',
            field=models.DateTimeField(default=datetime.datetime(2017, 12, 30, 14, 13, 43, 962420)),
        ),
    ]
