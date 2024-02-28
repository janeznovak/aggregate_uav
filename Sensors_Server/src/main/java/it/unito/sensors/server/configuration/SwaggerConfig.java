package it.unito.sensors.server.configuration;

import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

import springfox.documentation.builders.PathSelectors;
import springfox.documentation.builders.RequestHandlerSelectors;
import springfox.documentation.spi.DocumentationType;
import springfox.documentation.spring.web.plugins.Docket;
import springfox.documentation.swagger2.annotations.EnableSwagger2;

/**
 * \brief Configuration of Swagger, used to crate REST descriptor.
 *
 */
@Configuration
@EnableSwagger2
public class SwaggerConfig {

    static final String PACKAGE_CONTROLLER = "it.unito.sensors.server.controller";

    @Bean
    public Docket api() {
        return new Docket(DocumentationType.SWAGGER_2)
          .select()
          .apis(RequestHandlerSelectors.basePackage(PACKAGE_CONTROLLER))
          .paths(PathSelectors.any())
          .build();
    }
}