from django.urls import path

from web.views import index, statistics

app_name = "web"
urlpatterns = [
    path('', index, name="index"),
    path('statistics/', statistics, name="statistics"),
]
