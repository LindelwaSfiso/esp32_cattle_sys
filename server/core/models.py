from django.db import models


class Cow(models.Model):
    rfid = models.CharField(max_length=50)
    breed = models.IntegerField(
        choices=(
            (1, "Humpless Longhorns"), (2, "Humpless Shorthorns"), (3, "Large East African Zebu"),
            (4, "Small East African Zebu"), (5, "West African Zebu"), (6, "East African Sanga"),
            (7, "South African Sanga"), (8, "Zenga"), (9, "Other")
        ), default=7
    )
    gender = models.IntegerField(choices=((1, "Male"), (2, "Female")), default=1)


class WeightRecording(models.Model):
    cow = models.ForeignKey(Cow, related_name="weights", on_delete=models.CASCADE)
    weight = models.DecimalField(decimal_places=3, max_digits=20)
    updated_at = models.DateTimeField(auto_now=True)
    created_at = models.DateTimeField(auto_now_add=True)


# class SysError(models.Model):
#     """
#     Log system errors
#     """
#     cow = models.ForeignKey(Cow, related_name="errors", on_delete=models.CASCADE)
#     error = models.TextField()
#     created_at = models.DateTimeField(auto_now_add=True)
