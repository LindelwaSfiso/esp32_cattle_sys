from django.contrib.auth.decorators import login_required
from django.shortcuts import render, get_object_or_404

from core.models import WeightRecording, Cow


@login_required(login_url="admin:login")
def index(request):
	records = WeightRecording.objects.order_by('-created_at')[:10].prefetch_related('cow')
	return render(request, "web/index.html", {'records': records})


@login_required(login_url="admin:login")
def statistics(request):
	return render(request, "web/statistics.html", {})


def cow_detail(request, rfid):
	cow = get_object_or_404(Cow, rfid=rfid)
	cow_weights = WeightRecording.objects.filter(cow=cow).order_by('created_at')

	# Prepare data for ApexCharts
	# Ensure dates are in a format ApexCharts can use (e.g., ISO string or milliseconds)
	chart_dates = [record.created_at.isoformat() for record in cow_weights]
	chart_weights = [float(record.weight) for record in cow_weights]

	context = {
		'cow': cow,
		'chart_dates': chart_dates,
		'chart_weights': chart_weights,
	}
	return render(request, 'web/cow_detail.html', context)
