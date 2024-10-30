from django.urls import path

from api.views import upload, get_recent_data

app_name = "api"
urlpatterns = [
    path("upload/", upload, name="upload"),
    path("recent-data/", get_recent_data, name="recent-data"),
]
