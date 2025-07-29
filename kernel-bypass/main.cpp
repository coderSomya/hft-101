#include <rte_mbuf.h>
#include <rte_mempool.h>
#include <rte_ring.h>
#include <rte_eal.h>
#include <rte_launch.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_log.h>
#include <rte_eal_poc.h>
#include <rte_eal_memconfig.h>
#include <rte_eal_mempool.h>

#include <stdio.h>
#include <stdint.h>

#include <mtcp_api.h>  // tcp stack built on top of dpdk


#define MAX_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32

static const struct rte_eth_conf port_conf_default = {
    .rxmode = { .max_rx_pkt_len = RTE_ETHER_MAX_LEN },
};


struct rte_mempool *mbuf_pool;

//step 1: dpdk init
int dpdk_init(int args, char *argv[]){
    int ret = rte_eal_init(args, argv);
    if (ret < 0) {
        rte_exit(EXIT_FAILURE, "Error with EAL initialization\n");
    }
    return ret;
}

//step 2:  port init
void port_init(uint8_t port, uint8_t rx_rings, uint8_t tx_rings){
    struct rte_eth_conf port_conf = port_conf_default;
    const uint16_t rx_rings = 1, tx_rings = 1;
    int retval;

    const rte_eth_dev_info dev_info;
    rte_eth_dev_info_get(port, &dev_info);

    retval = rte_eth_dev_configure(port, rx_rings, tx_rings, &port_conf);
    if (retval < 0) {
        rte_exit(EXIT_FAILURE, "Error: Failed to configure port %d\n", port);
    }

    retval = rte_eth_rx_queue_setup(port, 0,128, rte_eth_dev_socket_id(port), NULL, mbuf_pool);
    if (retval < 0) {
        rte_exit(EXIT_FAILURE, "Error: RX queue setup failed\n");
    }

    retval = rte_eth_tx_queue_setup(port, 0, 128, rte_eth_dev_socket_id(port), NULL);
    if (retval < 0) {
        rte_exit(EXIT_FAILURE, "Error: TX queue setup failed\n");
    }

    retval = rte_eth_dev_start(port);
    if (retval < 0) {
        rte_exit(EXIT_FAILURE, "Error: Failed to start port %d\n", port);
    }

    rte_eth_promiscuous_enable(port);
}

//step 3: sending a tcp message
void send_tcp_message(const char* server_ip, uint16_t server_port){
    int sockfd;
    struct sockaddr_in addr;

    mtcp_init("mtcp.conf");

    int core = 0;
    mctx_t mctx = mtcp_create_context(core);

    sockfd = mtcp_socket(mctx, AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        return;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &addr.sin_addr);

    if(mtcp_connect(mctx, sockfd, (struct sockaddr*)&addr, sizeof(addr))< 0){
        perror("connection failed");
        return;
    }

    const char* msg = "Hello from dpdk";
    mtcp_write(mctx, sockfd, msg, strlen(msg));    
    
}

//step 4: receiving a udp message
void reciev_udp_packet(uint8_t port){
    struct rte_mbuf *bufs[BURST_SIZE];
    uint16_t nb_rx;

    while(1){
        // recieve a burst of packets
        nb_rx = rte_eth_rx_burst(port, 0, pkts, BURST_SIZE);

        if (nb_rx == 0) continue;

        for (int i = 0; i < nb_rx; i++){
            struct rte_mbuf *pkt = bufs[i];
            
            //extrac5 the udp payload manually
            struct rte_udp_hdr *udp_hdr = rte_pktmbuf_mtod(pkt, struct rte_udp_hdr*);
            uint16_t udp_len = ntohs(udp_hdr->dgram_len);
            uint8_t *udp_payload = rte_pktmbuf_mtod(pkt, uint8_t*) + sizeof(struct rte_udp_hdr);

            printf("Received UDP packet: %s\n", udp_payload);

            rte_pktmbuf_free(pkt);
        }
    }
}