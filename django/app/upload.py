from django.conf.urls import url

from . import views

urlpatterns = [
#	url(r'^$', views.index, name='index'),
#	url(r'^setari/', views.settings, name='settings'),
#	url(r'^$', views.rasadnic, name='rasadnic'),
	url(r'^$', views.upload, name='upload'),
#	url(r'^modul/(?P<Number>[0-9]+)$', views.module, name='module'),
#	url(r'^log/(?P<Number>[0-9]+)$', views.details, name='log'),
#	url(r'^rasadnic/setari/', views.settings, name='settings'),
#	url(r'^setari/(?P<Number>[0-9]+)$', views.settingsNo, name='settings'),
]
