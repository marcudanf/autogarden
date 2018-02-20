from django.conf.urls import url

from . import views

urlpatterns = [
    url(r'^$', views.index, name='index'),
	url(r'^upload/',views.upload, name='upload'),
	url(r'^module/(?P<Number>[0-9]+)$',views.module, name='module'),
	url(r'^log/(?P<Number>[0-9]+)$', views.details, name='log'),
	url(r'^settings/', views.settings, name='settings'),
]
