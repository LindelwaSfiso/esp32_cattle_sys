from django.contrib.auth.decorators import login_required
from django.shortcuts import render

from core.models import WeightRecording


@login_required(login_url="admin:login")
def index(request):
	records = WeightRecording.objects.all()[:10].prefetch_related('cow')
	return render(request, "web/index.html", {'records': records})


@login_required(login_url="admin:login")
def statistics(request):
	return render(request, "web/statistics.html", {})
