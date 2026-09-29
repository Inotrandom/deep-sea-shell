#ifndef DSS_EVENT_H
#define DSS_EVENT_H

#include <stdlib.h>

#define DSS_EVENT_MAX_CONNECTED 256

typedef void (*DSS_event_funcptr_t)(void *arg);

typedef struct
{
	DSS_event_funcptr_t *connected;
} DSS_event_t;

DSS_event_funcptr_t *DSS_event_connected_end(DSS_event_t *this) { return (this->connected + DSS_EVENT_MAX_CONNECTED); }

DSS_event_funcptr_t *DSS_event_connected_first_space(DSS_event_t *this)
{
	for (DSS_event_funcptr_t *iter = this->connected; iter < DSS_event_connected_end(this); ++iter)
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

	return DSS_event_connected_end(this);
}

unsigned int DSS_event_connect(DSS_event_t *this, DSS_event_funcptr_t func)
{
	unsigned int res;

	DSS_event_funcptr_t *space = DSS_event_connected_first_space(this);
	*space = func;

	res = (space - this->connected);

	return res;
}

void DSS_event_disconnect(DSS_event_t *this, const unsigned int idx) { this->connected[idx] = NULL; }

void DSS_event_fire(DSS_event_t *this, void *arg)
{
	for (DSS_event_funcptr_t *iter = this->connected; iter < DSS_event_connected_end(this); ++iter)
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
	DSS_event_t *this = (DSS_event_t *)malloc(sizeof(DSS_event_t));
	this->connected = (DSS_event_funcptr_t *)malloc(sizeof(DSS_event_funcptr_t *) * DSS_EVENT_MAX_CONNECTED);
	for (DSS_event_funcptr_t *iter = this->connected; iter < (this->connected + DSS_EVENT_MAX_CONNECTED); ++iter)
	{
		*iter = NULL;
	}

	return this;
}

void DSS_event_destroy(DSS_event_t *this)
{
	free(this);
	this = NULL;
}

#endif // DSS_EVENT_H
