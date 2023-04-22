package com.facebook.upb;

import static com.google.common.truth.Truth.assertThat;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import upb_test.Tiny.BabySteps;

@RunWith(JUnit4.class)
public class TinyProtoTest {
  @Test
  public void defaultInstance() throws Exception {
    BabySteps babySteps = BabySteps.getDefaultInstance();
    assertThat(babySteps.hasFoo()).isFalse();
    assertThat(babySteps.getFoo()).isEqualTo(43);
    assertThat(babySteps.hasBar()).isFalse();
    assertThat(babySteps.getBar()).isEqualTo(0);
  }

  @Test
  public void instantiation() throws Exception {
    BabySteps babySteps = BabySteps.newBuilder().setFoo(123).setBar(17).build();
    assertThat(babySteps.hasFoo()).isTrue();
    assertThat(babySteps.getFoo()).isEqualTo(123);
    assertThat(babySteps.hasBar()).isTrue();
    assertThat(babySteps.getBar()).isEqualTo(17);

    babySteps = BabySteps.newBuilder().build();
    assertThat(babySteps.hasFoo()).isFalse();
    assertThat(babySteps.getFoo()).isEqualTo(43);
    assertThat(babySteps.hasBar()).isFalse();
    assertThat(babySteps.getBar()).isEqualTo(0);
  }

  @Test
  public void reuseBuilder() throws Exception {
    BabySteps.Builder babyStepsBuilder = BabySteps.newBuilder().setFoo(123);

    BabySteps babySteps1 = babyStepsBuilder.build();
    assertThat(babySteps1.hasFoo()).isTrue();
    assertThat(babySteps1.getFoo()).isEqualTo(123);
    assertThat(babySteps1.hasBar()).isFalse();
    assertThat(babySteps1.getBar()).isEqualTo(0);

    babyStepsBuilder.setBar(17);

    BabySteps babySteps2 = babyStepsBuilder.build();
    assertThat(babySteps2.hasFoo()).isTrue();
    assertThat(babySteps2.getFoo()).isEqualTo(123);
    assertThat(babySteps2.hasBar()).isTrue();
    assertThat(babySteps2.getBar()).isEqualTo(17);

    // Test that changes to babyStepsBuilder after babySteps1 was created from it
    // are not reflected in babySteps1.
    assertThat(babySteps1.hasBar()).isFalse();
    assertThat(babySteps1.getBar()).isEqualTo(0);
  }

  @Test
  public void encodeDecode() throws Exception {
    BabySteps original = BabySteps.newBuilder().setFoo(123).setBar(17).build();
    
    byte[] data = original.toByteArray();
    assertThat(data).isNotNull();

    BabySteps decoded = BabySteps.parseFrom(data);

    assertThat(decoded.hasFoo()).isTrue();
    assertThat(decoded.getFoo()).isEqualTo(123);
    assertThat(decoded.hasBar()).isTrue();
    assertThat(decoded.getBar()).isEqualTo(17);
  }
}
