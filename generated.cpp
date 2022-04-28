#include "cpx.hh"
#include "avro/Encoder.hh"
#include "avro/Decoder.hh"

#include <memory>


int main()
{
    std::ostringstream oss;
    auto out = avro::ostreamOutputStream(oss, 32);
    avro::EncoderPtr e = avro::binaryEncoder();
    e->init(*out);
    c::cpx c1;
    c1.re = 1.0;
    c1.im = 2.13;
    avro::encode(*e, c1);
    out->flush();

    auto msg = oss.str();
    std::istringstream iss(msg);
    auto in = avro::istreamInputStream(iss, 32);
    avro::DecoderPtr d = avro::binaryDecoder();
    d->init(*in);
    c::cpx c2;
    avro::decode(*d, c2);
    std::cout << '(' << c2.re << ", " << c2.im << ')' << std::endl;
    return 0;
}