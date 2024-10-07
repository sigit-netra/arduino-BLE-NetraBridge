#pragma once

class taskHandler {
  private:
    /* data */
  public:
    virtual bool initialize () = 0;
    virtual void entrypoint () = 0;
    virtual void start ()      = 0;
    static void HandlerWrapper (void* _this);
};
