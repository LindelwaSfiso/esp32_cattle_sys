from django.urls import path

from web.views import index, statistics, recordings

app_name = "web"
urlpatterns = [
    path('', index, name="index"),
    path('statistics/', statistics, name="statistics"),
    path('recordings/', recordings, name="recordings"),
]
