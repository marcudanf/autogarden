from django.conf.urls import url

from . import views

urlpatterns = [
	url(r'^restart/(?P<Number>[0-9]+)$', views.restart, name='restart'),
	url(r'^log/(?P<Number>[0-9]+)$', views.details, name='log'),
	url(r'^$', views.rasadnic, name='rasadnic'),
	url(r'^modul/(?P<Number>[0-9]+)', views.module, name='module'),
	url(r'^log/(?P<Number>[0-9]+)', views.details, name='log'),
	url(r'^setari/(?P<Number>[0-9]+)', views.settingsNo, name='settings'),
	url(r'^setari', views.settings, name='settings'),

]
