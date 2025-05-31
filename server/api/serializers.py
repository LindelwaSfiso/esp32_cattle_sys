from rest_framework import serializers

from core.models import Cow, WeightRecording


class UploadSerializer(serializers.Serializer):
	rfid = serializers.CharField(required=True)
	weight = serializers.DecimalField(decimal_places=3, max_digits=20)


class CowSerializer(serializers.ModelSerializer):
	breed_display = serializers.CharField(source='get_breed_display', read_only=True)
	date_of_birth_formatted = serializers.SerializerMethodField(read_only=True)

	class Meta:
		model = Cow
		fields = '__all__'

	def get_date_of_birth_formatted(self, obj):
		return obj.date_of_birth.strftime('%m/%Y')


class WeightRecordingSerializer(serializers.ModelSerializer):
	cow = CowSerializer(read_only=True)

	class Meta:
		model = WeightRecording
		fields = '__all__'
