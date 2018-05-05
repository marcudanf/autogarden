# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations
import datetime


class Migration(migrations.Migration):

    dependencies = [
        ('app', '0026_auto_20180427_1844'),
    ]

    operations = [
        migrations.AlterField(
            model_name='general_module',
            name='pub_date',
            field=models.DateTimeField(default=datetime.datetime(2018, 4, 29, 12, 46, 50, 641183)),
            preserve_default=True,
        ),
    ]
