from django.db import models
from django.utils import timezone


class Cow(models.Model):
	BREED_CHOICES = (
		(1, "Humpless Longhorns"), (2, "Humpless Shorthorns"), (3, "Large East African Zebu"),
		(4, "Small East African Zebu"), (5, "West African Zebu"), (6, "East African Sanga"),
		(7, "South African Sanga"), (8, "Zenga"), (9, "Other")
	)
	FEEDING_SCHEME_CHOICES = (
		('PASTURE', 'Pasture Grazing'),
		('SUPPLEMENTED_PASTURE', 'Pasture + Supplement'),
		('FEEDLOT', 'Feedlot Intensive'),
		('MIXED', 'Mixed Diet'),
		('OTHER', 'Other')
	)

	rfid = models.CharField(max_length=50)
	breed = models.IntegerField(
		choices=BREED_CHOICES, default=7
	)
	gender = models.IntegerField(choices=((1, "Male"), (2, "Female")), default=1)
	date_of_birth = models.DateField(default=timezone.now)
	feeding_scheme = models.CharField(
		max_length=100,
		choices=FEEDING_SCHEME_CHOICES,
		default='PASTURE',
		help_text="The feeding regimen for this cow."
	)

	def __str__(self):
		return f"{self.rfid}: {self.get_breed_display()} - {self.date_of_birth.strftime('%m/%Y')}"


class WeightRecording(models.Model):
	cow = models.ForeignKey(Cow, related_name="weights", on_delete=models.CASCADE)
	weight = models.DecimalField(decimal_places=3, max_digits=20)
	updated_at = models.DateTimeField(auto_now=True)
	created_at = models.DateTimeField(auto_now_add=True)
