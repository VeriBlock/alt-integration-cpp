#ifndef F3EDAD40_0BA9_4727_9CCA_2FFC68D4E501
#define F3EDAD40_0BA9_4727_9CCA_2FFC68D4E501

namespace altintegration {
namespace abfi {
namespace btc {

struct P2pMessageHandler {
  virtual ~P2pMessageHandler() = default;
  virtual void onVersion()
};

}  // namespace btc
}  // namespace abfi
}  // namespace altintegration

#endif /* F3EDAD40_0BA9_4727_9CCA_2FFC68D4E501 */
