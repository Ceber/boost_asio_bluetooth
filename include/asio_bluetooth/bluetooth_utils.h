

#ifndef ANDROID
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

#include <sdbus-c++/sdbus-c++.h>

std::pair<sdp_session_t *, sdp_record_t *> register_service(uint8_t rfcomm_channel) {
  uint32_t service_uuid_int[] = {0x1101, 0, 0, 0xABCD};
  const char *service_name = "Bluetooth Serial Port";
  const char *service_dsc = "Serial RFCOMM Server channel 12";
  const char *service_prov = "CeDeV";

  uuid_t root_uuid, l2cap_uuid, rfcomm_uuid, svc_uuid, spp_uuid;
  sdp_list_t *l2cap_list = 0, *rfcomm_list = 0, *root_list = 0, *proto_list = 0, *access_proto_list = 0;
  sdp_data_t *channel = 0;
  sdp_profile_desc_t profile;
  // Create a new service record
  sdp_record_t *record = sdp_record_alloc();
  sdp_session_t *session = 0;

  sdp_set_info_attr(record, service_name, service_prov, service_dsc);

  sdp_uuid16_create(&spp_uuid, SERIAL_PORT_SVCLASS_ID);
  sdp_list_t *spp_list = sdp_list_append(0, &spp_uuid);
  sdp_set_service_classes(record, spp_list);

  // Set the general service ID
  sdp_uuid128_create(&svc_uuid, &service_uuid_int);
  sdp_set_service_id(record, svc_uuid);

  char str[256] = "";
  sdp_uuid2strn(&svc_uuid, str, 256);
  printf("Registering UUID %s\n", str);

  // Make the service record publicly browsable
  sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
  root_list = sdp_list_append(0, &root_uuid);
  sdp_set_browse_groups(record, root_list);

  // Set l2cap information
  sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
  l2cap_list = sdp_list_append(0, &l2cap_uuid);
  proto_list = sdp_list_append(0, l2cap_list);

  // Set rfcomm information
  sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
  channel = sdp_data_alloc(SDP_UINT8, &rfcomm_channel);
  rfcomm_list = sdp_list_append(0, &rfcomm_uuid);
  sdp_list_append(rfcomm_list, channel);
  sdp_list_append(proto_list, rfcomm_list);
  access_proto_list = sdp_list_append(0, proto_list);
  sdp_set_access_protos(record, access_proto_list);

  // Set the language base attribute list
  sdp_lang_attr_t base_lang;
  base_lang.code_ISO639 = 0x656e;
  base_lang.encoding = 0x6a;
  base_lang.base_offset = 0x100;
  sdp_list_t *lang_list = sdp_list_append(0, &base_lang);
  sdp_set_lang_attr(record, lang_list);

  // Set the profile descriptor list
  sdp_uuid16_create(&root_uuid, SERIAL_PORT_PROFILE_ID);
  uint16_t version = 0x0100;
  profile.uuid = root_uuid;
  profile.version = version;
  sdp_list_t *profile_list = sdp_list_append(0, &profile);
  sdp_set_profile_descs(record, profile_list);

  // Attach protocol information to service record
  bdaddr_t bdaddr_any = {{0, 0, 0, 0, 0, 0}};
  bdaddr_t bdaddr_local = {{0, 0, 0, 0xff, 0xff, 0xff}};
  session = sdp_connect(&bdaddr_any, &bdaddr_local, SDP_RETRY_IF_BUSY);
  sdp_list_free(access_proto_list, 0);
  sdp_list_free(proto_list, 0);
  sdp_list_free(rfcomm_list, 0);
  sdp_list_free(l2cap_list, 0);
  sdp_list_free(root_list, 0);
  sdp_list_free(profile_list, 0);

  // Register the service
  if (sdp_record_register(session, record, 0) < 0) {
    std::cerr << "Error registering SDP record\n";
    sdp_record_free(record);
    return {nullptr, nullptr};
  }

  // Assuming svc_uuid is your UUID
  char uuid_str[37]; // 36 bytes for UUID string and 1 byte for null terminator
  sdp_uuid2strn(&svc_uuid, uuid_str, sizeof(uuid_str));

  // Now uuid_str contains the string representation of the UUID
  std::cout << "Service UUID: " << uuid_str << std::endl;

  return {session, record};
}

void unregister_service(std::pair<sdp_session_t *, sdp_record_t *> session) {
  sdp_record_unregister(session.first, 0);
  sdp_close(session.first);
  sdp_record_free(session.second);
}

/**
 * @brief This function can block a while. It should be handled and called in some async way.
 * @return std::map<std::string, std::string>
 */
std::map<std::string, std::string> scan() {
  inquiry_info *ii = NULL;
  int max_rsp, num_rsp;
  int dev_id, sock, len, flags;
  int i;
  char addr[19] = {0};
  char name[248] = {0};

  dev_id = hci_get_route(NULL);
  sock = hci_open_dev(dev_id);
  if (dev_id < 0 || sock < 0) {
    perror("opening socket");
    throw std::runtime_error("opening socket");
  }

  len = 8;
  max_rsp = 255;
  flags = IREQ_CACHE_FLUSH;
  ii = (inquiry_info *)malloc(max_rsp * sizeof(inquiry_info));

  num_rsp = hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);
  if (num_rsp < 0) {
    perror("hci_inquiry");
  }
  std::map<std::string, std::string> result;
  for (i = 0; i < num_rsp; i++) {

    ba2str(&(ii + i)->bdaddr, addr);
    memset(name, 0, sizeof(name));
    if (hci_read_remote_name(sock, &(ii + i)->bdaddr, sizeof(name), name, 0) < 0)
      strcpy(name, "[unknown]");
    printf("%s  %s\n", addr, name);
    result[name] = addr;
  }

  free(ii);
  close(sock);
  return result;
}

/**
 * @brief Scan for BLE devices.
 * @return std::map<std::string, std::string>
 */
std::map<std::string, std::string> scanBLE() {
  int dev_id, sock;
  char addr[19] = {0};
  char name[248] = {0};

  dev_id = hci_get_route(NULL);
  sock = hci_open_dev(dev_id);
  if (dev_id < 0 || sock < 0) {
    perror("opening socket");
    return {};
  }

  // Set fd non-blocking
  int on = 1;
  if (ioctl(sock, FIONBIO, (char *)&on) < 0) {
    perror("Could set device to non-blockings");
    return {};
  }

  // Set scan parameters
  if (hci_le_set_scan_parameters(sock, 0x01, htobs(0x0010), htobs(0x0010), 0x00, 0x00, 1000) < 0) {
    perror("Failed to set scan parameters");
    return {};
  }

  // Enable scan
  if (hci_le_set_scan_enable(sock, 0x01, 1, 1000) < 0) {
    perror("Failed to enable scan");
    return {};
  }

  // Create a set to store the BLE addresses
  std::map<std::string, std::string> ble_addresses;

  // Read the advertising reports
  struct hci_filter nf;
  hci_filter_clear(&nf);
  hci_filter_set_ptype(HCI_EVENT_PKT, &nf);
  hci_filter_set_event(EVT_LE_META_EVENT, &nf);
  if (setsockopt(sock, SOL_HCI, HCI_FILTER, &nf, sizeof(nf)) < 0) {
    perror("Could not set socket options\n");
  }

  // Buffer to store the advertising reports
  unsigned char buf[HCI_MAX_EVENT_SIZE];

  while (1) {
    int len = read(sock, buf, sizeof(buf));
    if (len < 0) {
      perror("Read failed");
      break;
    }

    // Parse the advertising report
    evt_le_meta_event *meta = (evt_le_meta_event *)(buf + (1 + HCI_EVENT_HDR_SIZE));
    le_advertising_info *info = (le_advertising_info *)(meta->data + 1);

    // Convert the address to a string
    ba2str(&info->bdaddr, addr);
    memset(name, 0, sizeof(name));
    if (hci_read_remote_name(sock, &(info->bdaddr), sizeof(name), name, 0) < 0)
      strcpy(name, "[unknown]");

    // Add the address and name to the set
    ble_addresses[name] = addr;
  }

  // Reset scan parameters
  if (hci_le_set_scan_parameters(sock, 0x00, htobs(0x0010), htobs(0x0010), 0x00, 0x00, 1000) < 0) {
    perror("Failed to reset scan parameters");
  }

  // Disable scan
  if (hci_le_set_scan_enable(sock, 0x00, 1, 1000) < 0) {
    perror("Failed to disable scan");
  }
  close(sock);
  return ble_addresses;
}

#endif