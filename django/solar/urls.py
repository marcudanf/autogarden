from django.conf.urls import url
from . import views

urlpatterns = [
	url(r'^$', views.solar, name='solar'),
	url(r'^upload/', views.upload, name='upload'),
	url(r'^(?P<Number>[0-9]+)', views.solarx, name='solarx'),
]
