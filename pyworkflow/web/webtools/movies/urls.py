import os
from django.conf.urls import url
import pyworkflow as pw
from pyworkflow.web.webtools.movies.views import MOVIES_SERVICE_URL
from pyworkflow.web.app.views_util import ownRedirect

MEDIA_MOVIES = os.path.join(pw.HOME, 'web', 'webtools', 'movies', 'resources')

urls = [
    (r'^resources_movies/(?P<path>.*)$', 
        'django.views.static.serve', 
        {'document_root': MEDIA_MOVIES}
    ),
    
    url(r'^' + MOVIES_SERVICE_URL + '$', 'app.views_webtools.service_movies'),
    url(r'^create_movies_project/$', 'app.views_webtools.create_movies_project'),
    url(r'^mov_form/$', 'app.views_webtools.movies_form'),
    url(r'^m_content$', 'app.views_webtools.movies_content', name='movies'),
    url(r'^m_content/$', ownRedirect, {'url': '../m_content', 'permanent': False}),

]