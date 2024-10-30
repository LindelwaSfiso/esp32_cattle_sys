from rest_framework import status
from rest_framework.decorators import api_view
from rest_framework.response import Response


@api_view(["POST"])
def upload(request):
    return Response({}, status=status.HTTP_200_OK)


@api_view(["GET"])
def get_recent_data(request):
    return Response({}, status=status.HTTP_200_OK)
