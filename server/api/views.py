from django.http import JsonResponse
from rest_framework import status
from rest_framework.decorators import api_view
from rest_framework.response import Response

from api.serializers import WeightRecordingSerializer
from core.models import WeightRecording, Cow

PEAK_COW_WEIGHT = 600 * 1000  # 600kg
SCALE_LIMIT_GRAMS = 10 * 1000  # 10kg


@api_view(["POST"])
def upload(request):
	"""
	API endpoint for the ESP32 to send RFID and weight data (in grams).
	"""
	rfid = request.data.get('rfid')
	weight_grams = request.data.get('weight')

	if not rfid or weight_grams is None:
		return Response({'status': 'error', 'message': 'Missing RFID or weight'}, status=status.HTTP_400_BAD_REQUEST)

	try:
		cow = Cow.objects.get(rfid=rfid)
	except Cow.DoesNotExist:
		return Response({'status': 'error', 'message': f'RFID "{rfid}" not found'}, status=status.HTTP_404_NOT_FOUND)

	try:
		weight_grams = float(weight_grams)
		# Apply the linear mapping formula to estimate cow weight
		estimated_cow_weight = (PEAK_COW_WEIGHT / SCALE_LIMIT_GRAMS) * weight_grams
		weight_kg = estimated_cow_weight  # For demonstration

		WeightRecording.objects.create(cow=cow, weight=weight_kg)
		return Response({'status': 'success', 'message': f'Weight: {weight_kg:.2f}kg'}, status=status.HTTP_201_CREATED)

	except ValueError:
		return Response({'status': 'error', 'message': 'Invalid weight format'}, status=status.HTTP_400_BAD_REQUEST)


@api_view(["GET"])
def get_recent_data(request):
	"""
	AJAX endpoint to get the latest weight recording.
	"""
	is_ajax = request.META.get('HTTP_X_REQUESTED_WITH') == 'XMLHttpRequest'
	if is_ajax or True:
		latest_recording = WeightRecording.objects.prefetch_related(
			'cow'
		).order_by('-created_at').all()[:10]
		if latest_recording:
			serializer = WeightRecordingSerializer(latest_recording, many=True)
			return JsonResponse({
				'message': "Latest weight data retrieved successfully.",
				'data': serializer.data
			}, status=200)
		else:
			return JsonResponse({
				'message': "No weight data available."
			}, status=404)  # Still a successful query, just no data

	return JsonResponse({
		'error': 'Method not allowed.'
	}, status=400)  # Changed to 400 for non-AJAX requests
