#include <sabi/conv.h>
#include <sabi/host.h>
#include <sabi/api.h>
#include <string.h>

int sabi_atoi(uint8_t c, int base)
{
	int value;

	if(c >= '0' && c <= '9')
	{
		value = (c - '0');
	}
	else if(c >= 'A' && c <= 'Z')
	{
		value = (c - 'A' + 10);
	}
	else if(c >= 'a' && c <= 'z')
	{
		value = (c - 'a' + 10);
	}
	else
	{
		value = -1;
	}

	if(value < base)
	{
		return value;
	}

	return -1;
}

int sabi_itoa(char *dest, uint64_t value, int base)
{
	const char *list = "0123456789ABCDEF";
	char *ptr = dest;
	int tmp, len;

	do
	{
		*ptr++ = list[value % base];
		value = (value / base);
	}
	while(value);

	len = (ptr - dest);
	*ptr-- = '\0';

	while(dest < ptr)
	{
		tmp = *dest;
		*dest++ = *ptr;
		*ptr-- = tmp;
	}

	return len;
}

void sabi_conv_tobuffer(sabi_data_t *dptr, sabi_data_t *sptr)
{
	void *ptr;
	int len;

	if(sptr->type == SABI_DATA_BUFFER)
	{
		ptr = sptr->buffer.ptr;
		len = sptr->buffer.size;
	}
	else if(sptr->type == SABI_DATA_INTEGER)
	{
		ptr = &(sptr->integer.value);
		len = 8;
	}
	else if(sptr->type == SABI_DATA_STRING)
	{
		ptr = sptr->string.value;
		len = strlen(ptr);
		if(len)
		{
			len++;
		}
	}
	else
	{
		len = 0;
	}

	dptr->buffer.type = SABI_DATA_BUFFER;
	dptr->buffer.size = len;
	dptr->buffer.ptr = 0;
	if(len)
	{
		dptr->buffer.ptr = sabi_host_alloc(1, len);
		memcpy(dptr->buffer.ptr, ptr, len);
	}
}

void sabi_conv_tointeger(sabi_data_t *dptr, sabi_data_t *sptr)
{
	int base, len, tmp;
	uint64_t value;
	char *str;

	value = 0;
	base = 10;

	if(sptr->type == SABI_DATA_INTEGER)
	{
		value = sptr->integer.value;
	}
	else if(sptr->type == SABI_DATA_BUFFER)
	{
		len = sptr->buffer.size;

		if(len > 8)
		{
			len = 8;
		}

		while(len--)
		{
			value <<= 8;
			value |= sptr->buffer.ptr[len];
		}
	}
	else if(sptr->type == SABI_DATA_STRING)
	{
		str = sptr->string.value;
		len = strlen(str);

		if(len >= 2)
		{
			if(str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
			{
				base = 16;
				str += 2;
				len -= 2;
			}
		}

		while(len--)
		{
			tmp = sabi_atoi(*str++, base);
			if(tmp < 0)
			{
				break;
			}
			value = (value * base) + tmp;
		}
	}
	else
	{
		value = 0;
	}

	dptr->integer.type = SABI_DATA_INTEGER;
	dptr->integer.value = value;
}

void sabi_conv_tostring(sabi_data_t *dptr, sabi_data_t *sptr, int maxlen)
{
	void *ptr;
	int len;

	if(sptr->type == SABI_DATA_STRING)
	{
		ptr = sptr->string.value;
		len = strlen(ptr);
	}
	else if(sptr->type == SABI_DATA_BUFFER)
	{
		ptr = sptr->buffer.ptr;
		len = sptr->buffer.size;
	}
	else if(sptr->type == SABI_DATA_INTEGER)
	{
		ptr = &(sptr->integer.value);
		len = 8;
	}
	else
	{
		len = 0;
	}

	if((maxlen > 0) && (maxlen < len))
	{
		len = maxlen;
	}

	dptr->string.type = SABI_DATA_STRING;
	// allocate len+1 bytes
	strncpy(dptr->string.value, ptr, len);
	dptr->string.value[len] = '\0';
}

void sabi_conv_tobasestring(sabi_data_t *dptr, sabi_data_t *sptr, int base)
{
	int len, comma;
	uint8_t *buf;
	char *str;

	if(sptr->type == SABI_DATA_STRING)
	{
		sabi_clone_data(dptr, sptr);
	}
	else if(sptr->type == SABI_DATA_INTEGER)
	{
		len = (19 + 1);

		str = sabi_host_alloc(1, len);
		dptr->string.type = SABI_DATA_STRING;
		//dptr->string.value = str;

		sabi_itoa(str, sptr->integer.value, base);
		strcpy(dptr->string.value, str); // temporary
	}
	else if(sptr->type == SABI_DATA_BUFFER)
	{
		len = (5 * sptr->buffer.size) + 1;

		str = sabi_host_alloc(1, len);
		dptr->string.type = SABI_DATA_STRING;
		//dptr->string.value = str;
		char *tmp = str; // temporary

		comma = 0;
		len = sptr->buffer.size;
		buf = sptr->buffer.ptr;

		while(len--)
		{
			if(comma)
			{
				*str++ = ',';
			}

			if(base == 16)
			{
				*str++ = '0';
				*str++ = 'x';
			}

			str += sabi_itoa(str, *buf++, base);
			comma = 1;
		}

		strcpy(dptr->string.value, tmp); // temporary
	}
}
