# -*- coding: utf-8 -*-
# Generated by Django 1.11.7 on 2018-01-02 11:32
from __future__ import unicode_literals

import datetime
from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('app', '0013_auto_20171230_1745'),
    ]

    operations = [
        migrations.AddField(
            model_name='general_module',
            name='count',
            field=models.IntegerField(default=0),
        ),
        migrations.AlterField(
            model_name='general_module',
            name='pub_date',
            field=models.DateTimeField(default=datetime.datetime(2018, 1, 2, 13, 32, 31, 811902)),
        ),
    ]
