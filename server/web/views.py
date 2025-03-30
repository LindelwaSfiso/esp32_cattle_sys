from django.contrib.auth.decorators import login_required
from django.shortcuts import render

from core.models import Cow, WeightRecording


@login_required(login_url="admin:login")
def index(request):
	return render(request, "web/index.html", {})


@login_required(login_url="admin:login")
def statistics(request):
	return render(request, "web/statistics.html", {})


@login_required(login_url="admin:login")
def recordings(request):
	# get the most recent 10 records
	records = WeightRecording.objects.all()[:10]
	is_empty = not records.exists()
	return render(request, "web/recordings.html", {'records': records, 'empty': is_empty})
