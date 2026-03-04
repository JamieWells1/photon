#include <dns_server.h>

#include <const.h>
#include <macro.h>

#include <stdio.h>
#include <string.h>

#include <lwip/udp.h>
#include <pico/stdlib.h>

static struct udp_pcb* dns_pcb = NULL;

static void dns_recv_callback(void* arg, struct udp_pcb* pcb, struct pbuf* p, const ip_addr_t* addr, u16_t port)
{
    if (p->len < 12) {
        pbuf_free(p);
        return;
    }

    uint8_t* data = (uint8_t*)p->payload;

    uint16_t flags = (data[2] << 8) | data[3];
    uint16_t questions = (data[4] << 8) | data[5];

    if ((flags & 0x8000) || questions == 0) {
        pbuf_free(p);
        return;
    }

    uint8_t response[512];
    memset(response, 0, sizeof(response));

    response[0] = data[0];
    response[1] = data[1];
    response[2] = 0x81;
    response[3] = 0x80;
    response[4] = data[4];
    response[5] = data[5];
    response[6] = 0x00;
    response[7] = 0x01;
    response[8] = 0x00;
    response[9] = 0x00;
    response[10] = 0x00;
    response[11] = 0x00;

    int question_len = p->len - 12;
    if (question_len > 400) question_len = 400;
    memcpy(&response[12], &data[12], question_len);

    int answer_offset = 12 + question_len;

    response[answer_offset++] = 0xC0;
    response[answer_offset++] = 0x0C;
    response[answer_offset++] = 0x00;
    response[answer_offset++] = 0x01;
    response[answer_offset++] = 0x00;
    response[answer_offset++] = 0x01;
    response[answer_offset++] = 0x00;
    response[answer_offset++] = 0x00;
    response[answer_offset++] = 0x00;
    response[answer_offset++] = 0x3C;
    response[answer_offset++] = 0x00;
    response[answer_offset++] = 0x04;
    response[answer_offset++] = 192;
    response[answer_offset++] = 168;
    response[answer_offset++] = 4;
    response[answer_offset++] = 1;

    struct pbuf* response_pbuf = pbuf_alloc(PBUF_TRANSPORT, answer_offset, PBUF_RAM);
    if (response_pbuf)
    {
        memcpy(response_pbuf->payload, response, answer_offset);
        udp_sendto(pcb, response_pbuf, addr, port);
        pbuf_free(response_pbuf);
    }

    pbuf_free(p);
}

void dns_server_start(void)
{
    if (dns_pcb) return;

    dns_pcb = udp_new();
    if (!dns_pcb)
    {
        debug(ERROR, "Failed to create DNS PCB");
        return;
    }

    udp_bind(dns_pcb, IP_ADDR_ANY, 53);
    udp_recv(dns_pcb, dns_recv_callback, NULL);

    debug(INFO, "DNS server started on port 53");
}

void dns_server_stop(void)
{
    if (dns_pcb)
    {
        udp_remove(dns_pcb);
        dns_pcb = NULL;
        debug(INFO, "DNS server stopped");
    }
}
