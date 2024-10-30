from django.contrib import admin
from core.models import Cow, WeightRecording


@admin.register(Cow)
class CowAdmin(admin.ModelAdmin):
    list_per_page = 25
    list_display = ["id", "rfid", "breed_str", "gender"]

    def breed_str(self, cow):
        return cow.get_breed_display()


@admin.register(WeightRecording)
class WeightRecordingAdmin(admin.ModelAdmin):
    list_per_page = 25
    list_display = ["id", "cow", "weight", "created_at"]


admin.site.site_header = 'Cattle Weighting System'
admin.site.site_title = 'CattleSys'
admin.site.index_title = 'Cattle Sys Administration'
