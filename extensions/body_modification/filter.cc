#include <string>
#include "proxy_wasm_intrinsics.h"
#include "absl/strings/str_cat.h" 
#include "rapidjson/document.h"

using namespace rapidjson;
using namespace std;

constexpr StringView ModifyBodyHeader = "x-body-modification";
constexpr char ModifyPosition[] = "position";
constexpr char ModifyValue[] = "value";
constexpr char ModifyType[] = "type";
constexpr char ModifyOperation[] = "operation";

struct ModifyBodyOp {
      string postion;
      string value;
      string type;
      string operation;
};

class ModifyBodyContext : public Context {
public:
      explicit ModifyBodyContext(uint32_t id, RootContext* root) : Context(id, root) {}

      void onCreate() override;
      FilterHeadersStatus onRequestHeaders(uint32_t) override;
      FilterDataStatus onRequestBody(size_t body_buffer_length, bool end_of_stream) override;
      void onDone() override;
      void onLog() override;
      void onDelete() override;
private:
      void setModifyBodyOps(std::string ops);
      vector<ModifyBodyOp> modify_body_ops_;
};

void ModifyBodyContext::setModifyBodyOps(string op_str) {
      logDebug(absl::StrCat("x-body-modification value: ", op_str));
      Document d;
      d.Parse(op_str.c_str());
      if (d.HasParseError()) {
            logWarn("unable to parse x-body-modification value");
            return;
      } 

      for(SizeType i = 0; i < d.Size(); ++i) {
            string postion = "", value = "", type = "", operation = "";
            if (d[i].HasMember(ModifyPosition) && d[i][ModifyPosition].IsString()) {
                  postion = d[i][ModifyPosition].GetString();
            }
            if (d[i].HasMember(ModifyValue) && d[i][ModifyValue].IsString()) {
                  value = d[i][ModifyValue].GetString();
            }
            if (d[i].HasMember(ModifyType) && d[i][ModifyType].IsString()) {
                  type = d[i][ModifyType].GetString();
            }
            if (d[i].HasMember(ModifyOperation) && d[i][ModifyOperation].IsString()) {
                  operation = d[i][ModifyOperation].GetString();
            }
            ModifyBodyOp op = {postion, value, type, operation};

            modify_body_ops_.push_back(op);
      }
      return;
}

void ModifyBodyContext::onCreate() { logWarn("onCreate" + to_string(id())); }

FilterHeadersStatus ModifyBodyContext::onRequestHeaders(uint32_t s) {
      logDebug(string("onRequestHeaders ") + to_string(id()));

      auto modify_body_value = getRequestHeader(ModifyBodyHeader);
      if (modify_body_value != nullptr && !modify_body_value->view().empty()) {
	    removeRequestHeader(ModifyBodyHeader);
            setModifyBodyOps(modify_body_value->toString());
      }
      return FilterHeadersStatus::Continue;
}

FilterDataStatus ModifyBodyContext::onRequestBody(size_t body_buffer_length, bool end_of_stream) {
      auto body = getBufferBytes(BufferType::HttpRequestBody, 0, body_buffer_length);
      logError(string("onRequestBody ") + string(body->view()));
      return FilterDataStatus::Continue;
}

void ModifyBodyContext::onDone() {
      logWarn("onDone " + to_string(id()));
}

void ModifyBodyContext::onLog() {
      logWarn("onLog " + to_string(id()));
}

void ModifyBodyContext::onDelete() {
      logWarn("onDelete " + to_string(id()));
}
                            
