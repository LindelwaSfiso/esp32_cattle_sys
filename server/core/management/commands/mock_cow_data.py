# core/management/commands/mock_cow_data.py
from django.core.management.base import BaseCommand
from core.models import Cow, WeightRecording  # Adjust 'core' to your app name
from django.utils import timezone
import datetime
import random


class Command(BaseCommand):
	help = 'Mocks cow weight data for a single cow from birth to middle age to simulate growth.'

	def handle(self, *args, **options):
		self.stdout.write(self.style.SUCCESS('Starting mock data generation...'))

		# --- 1. Create or get a Cow instance ---
		# For demonstration, we'll create a new cow with a unique RFID each time
		# You can change this to target an existing cow if needed.
		rfid = f"COW_{random.randint(10000, 99999)}"
		# Start the cow's birth date 3 years ago to simulate growth up to middle age
		birth_date = timezone.now().date() - datetime.timedelta(days=random.choice([3, 2, 4]) * 365)

		cow, created = Cow.objects.get_or_create(
			rfid=rfid,
			defaults={
				'breed': random.choice([1, 2, 3, 4, 5, 6, 7, 8]),
				'gender': random.choice([1, 2]),  # Female
				'date_of_birth': birth_date,
				'feeding_scheme': random.choice(['PASTURE', 'SUPPLEMENTED_PASTURE', 'FEEDLOT', 'MIXED'])
				# Assign a feeding scheme
			}
		)

		if created:
			self.stdout.write(self.style.SUCCESS(f'Created new cow: {cow.rfid} ({cow.get_breed_display()})'))
		else:
			self.stdout.write(
				self.style.WARNING(f'Using existing cow: {cow.rfid}. Deleting existing weight records for this cow...'))
			WeightRecording.objects.filter(cow=cow).delete()

		# --- 2. Simulate Growth Data ---
		initial_weight_kg = 25.0  # Realistic birth weight for a Zebu calf
		# Target weight for a mature Large East African Zebu at 3 years (approx. middle age)
		# This is an estimation for simulation purposes.
		target_weight_at_3_years_kg = 450.0

		current_date = cow.date_of_birth
		end_date = current_date + datetime.timedelta(days=3 * 365)  # Simulate for 3 years

		current_weight = initial_weight_kg
		recordings_count = 0

		self.stdout.write(self.style.SUCCESS(f'Simulating growth for {cow.rfid} from {current_date} to {end_date}...'))

		while current_date <= end_date:
			age_in_days = (current_date - cow.date_of_birth).days

			# Phase 1: Rapid growth (first 12 months)
			if age_in_days <= 365:
				# Higher daily gain during calfhood
				daily_gain = random.uniform(0.8, 1.2)  # 0.8 to 1.2 kg/day
			# Phase 2: Slower, steady growth (12 to 36 months)
			else:
				# Slower daily gain as the cow approaches maturity
				daily_gain = random.uniform(0.2, 0.5)  # 0.2 to 0.5 kg/day

			current_weight += daily_gain

			# Add some natural daily weight fluctuations (e.g., due to hydration, digestion)
			# This makes the curve less perfectly smooth.
			current_weight += random.uniform(-0.5, 0.5)

			# Ensure weight doesn't drop unrealistically low, especially early on
			# This is a simple safeguard, more complex models exist.
			current_weight = max(current_weight, initial_weight_kg + (age_in_days * 0.05))

			# Record weight every 7 days (weekly) to keep the number of records manageable
			if age_in_days % 7 == 0:
				# Add some additional noise to the recorded weight for realism
				recorded_weight = round(current_weight + random.uniform(-7.0, 7.0), 3)
				# Ensure recorded weight doesn't go below a sensible minimum (e.g., birth weight)
				recorded_weight = max(recorded_weight, initial_weight_kg)

				WeightRecording.objects.create(
					cow=cow,
					weight=recorded_weight,
					# Set created_at to the current date for the recording
					created_at=timezone.datetime.combine(
						current_date,
						# Add random time of day for more realism
						datetime.time(random.randint(8, 18), random.randint(0, 59), random.randint(0, 59))
					)
				)
				recordings_count += 1

			current_date += datetime.timedelta(days=1)  # Move to the next day

		self.stdout.write(
			self.style.SUCCESS(f'Finished simulating data. Created {recordings_count} weight records for {cow.rfid}.'))
		self.stdout.write(self.style.SUCCESS(f'Final simulated weight for {cow.rfid}: {round(current_weight, 2)} kg'))
