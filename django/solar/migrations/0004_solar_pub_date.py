# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations
import datetime


class Migration(migrations.Migration):

    dependencies = [
        ('solar', '0003_remove_solar_set'),
    ]

    operations = [
        migrations.AddField(
            model_name='solar',
            name='pub_date',
            field=models.DateTimeField(default=datetime.datetime(2018, 4, 29, 12, 46, 50, 662685)),
            preserve_default=True,
        ),
    ]
