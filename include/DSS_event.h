#ifndef DSS_EVENT_H
#define DSS_EVENT_H

#include <stdlib.h>

#define DSS_EVENT_MAX_CONNECTED 256

typedef void (*DSS_event_funcptr_t)(void *arg);

typedef struct
{
	DSS_event_funcptr_t *connected;
} DSS_event_t;

DSS_event_funcptr_t *DSS_event_connected_end(DSS_event_t *self) { return (self->connected + DSS_EVENT_MAX_CONNECTED); }

DSS_event_funcptr_t *DSS_event_connected_first_space(DSS_event_t *self)
{
	for (DSS_event_funcptr_t *iter = self->connected; iter < DSS_event_connected_end(self); ++iter)
	{
		if (iter == NULL)
		{
			continue;
		}

		if (*iter != NULL)
		{
			continue;
		}

		return iter;
	}

	return DSS_event_connected_end(self);
}

unsigned int DSS_event_connect(DSS_event_t *self, DSS_event_funcptr_t func)
{
	unsigned int res;

	DSS_event_funcptr_t *space = DSS_event_connected_first_space(self);
	*space = func;

	res = (space - self->connected);

	return res;
}

void DSS_event_disconnect(DSS_event_t *self, const unsigned int idx) { self->connected[idx] = NULL; }

void DSS_event_fire(DSS_event_t *self, void *arg)
{
	for (DSS_event_funcptr_t *iter = self->connected; iter < DSS_event_connected_end(self); ++iter)
	{
		if (iter == NULL)
		{
			continue;
		}

		if (*iter == NULL)
		{
			continue;
		}

		(*iter)(arg);
	}
}

DSS_event_t *DSS_event_create(void)
{
	DSS_event_t *self = (DSS_event_t *)malloc(sizeof(DSS_event_t));
	self->connected = (DSS_event_funcptr_t *)malloc(sizeof(DSS_event_funcptr_t *) * DSS_EVENT_MAX_CONNECTED);
	for (DSS_event_funcptr_t *iter = self->connected; iter < (self->connected + DSS_EVENT_MAX_CONNECTED); ++iter)
	{
		*iter = NULL;
	}

	return self;
}

void DSS_event_destroy(DSS_event_t *self)
{
	free(self->connected);
	free(self);
	self = NULL;
}

#endif // DSS_EVENT_H
