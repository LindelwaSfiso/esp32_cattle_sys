# Generated by Django 5.1.2 on 2024-10-30 02:20

import django.db.models.deletion
from django.db import migrations, models


class Migration(migrations.Migration):

    initial = True

    dependencies = [
    ]

    operations = [
        migrations.CreateModel(
            name='Cow',
            fields=[
                ('id', models.BigAutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('rfid', models.CharField(max_length=50)),
                ('breed', models.IntegerField(choices=[(1, 'Humpless Longhorns'), (2, 'Humpless Shorthorns'), (3, 'Large East African Zebu'), (4, 'Small East African Zebu'), (5, 'West African Zebu'), (6, 'East African Sanga'), (7, 'South African Sanga'), (8, 'Zenga'), (9, 'Other')], default=7)),
                ('gender', models.CharField(choices=[(1, 'Male'), (2, 'Female')], default=1, max_length=50)),
            ],
        ),
        migrations.CreateModel(
            name='WeightRecording',
            fields=[
                ('id', models.BigAutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('weight', models.DecimalField(decimal_places=3, max_digits=20)),
                ('updated_at', models.DateTimeField(auto_now=True)),
                ('created_at', models.DateTimeField(auto_now_add=True)),
                ('cow', models.ForeignKey(on_delete=django.db.models.deletion.CASCADE, related_name='weights', to='core.cow')),
            ],
        ),
    ]
