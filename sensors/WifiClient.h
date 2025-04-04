class WifiClient {
  public:
    WifiClient(char *ssid, char *pass);
    void connect();
    int check_connection();

  private:
    char *_ssid;
    char *_pass;
};
