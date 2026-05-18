#ifndef SRT_ENVIRONMENT_H_
#define SRT_ENVIRONMENT_H_

class SrtEnvironment
{
public:
    SrtEnvironment();
    ~SrtEnvironment();

    SrtEnvironment(const SrtEnvironment&) = delete;
    SrtEnvironment& operator=(const SrtEnvironment&) = delete;
};

#endif /* SRT_ENVIRONMENT_H_ */