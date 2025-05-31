from django.urls import path

from web.views import index, statistics,cow_detail

app_name = "web"
urlpatterns = [
	path('', index, name="index"),
	path('statistics/', statistics, name="statistics"),
	path('cows/<str:rfid>/', cow_detail, name='cow_detail'),
]
