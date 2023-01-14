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
  }
}
