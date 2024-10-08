FROM ubuntu:jammy

ARG DEBIAN_FRONTEND=noninteractive

# install dependencies

RUN rm -rf /var/lib/apt/lists/*

# small workaround because of hash mismatch errors (dev problem)
RUN until apt-get update; do \
    echo "Retrying..."; \
    sleep 1; \
    done


RUN until apt-get install -y cmake; do \
    echo "Retrying..."; \
    sleep 1; \
    done
	

RUN until apt-get install -y iputils-ping; do \
    echo "Retrying..."; \
    sleep 1; \
    done
	
RUN until apt-get install -y net-tools; do \
    echo "Retrying..."; \
    sleep 1; \
    done

RUN until apt-get install -y curl; do \
    echo "Retrying..."; \
    sleep 1; \
    done
	
RUN until apt-get install -y python3; do \
    echo "Retrying..."; \
    sleep 1; \
    done
	
RUN until apt-get install -y python3-pip; do \
    echo "Retrying..."; \
    sleep 1; \
    done
	
RUN until apt-get install -y clang-format; do \
    echo "Retrying..."; \
    sleep 1; \
    done
	
RUN until apt-get install -y clang-tidy; do \
    echo "Retrying..."; \
    sleep 1; \
    done
	
RUN rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
COPY ["conanfile.py", "./"]

# so that we can use python command as well
RUN echo 'alias python=python3' >> ~/.bashrc

# install conan package manager
RUN pip install conan

RUN conan profile detect --force
RUN conan install . --build=missing

# only required to build missing dependencies.
# aftewards we do not want to mix that up with the conan cmake package
RUN apt-get remove -y cmake

# Set the default shell to Bash
SHELL ["/bin/bash", "-c"]
