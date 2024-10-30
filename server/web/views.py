from django.contrib.auth.decorators import login_required
from django.http import Http404
from django.shortcuts import render, get_object_or_404

from core.models import Cow


@login_required(login_url="admin:login")
def index(request):
    return render(request, "web/index.html", {})


@login_required(login_url="admin:login")
def cattle_list(request):
    return render(request, "web/cattle-list.html", {})


@login_required(login_url="admin:login")
def cow_details(request, pk: int):
    try:
        cow = get_object_or_404(Cow, id=pk)
        # todo: get more details like cattle statistics
        return render(request, "web/cow-details.html", {"cow": cow})
    except Http404:
        pass
    return render(request, "web/cow-details.html", {})
