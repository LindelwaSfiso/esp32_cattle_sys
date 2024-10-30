from django.urls import path

from web.views import index, cattle_list, cow_details

app_name = "web"
urlpatterns = [
    path('', index, name="index"),
    path('cattle/', cattle_list, name="cattle-list"),
    path('cattle/<int:pk>/', cow_details, name="cow-details"),
]
