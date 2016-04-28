<?php

/**
 * @file
 * Prp instagram feed admin page callaback functions.
 */

/**
 * Form builder; Instagram settings form.
 */
function custom_instagram_feed_admin_form($form, &$form_state) {
  $form['oauth'] = array(
    '#type' => 'fieldset',
    '#title' => t('Instagram OAuth Settings'),
    '#description' => t('To enable OAuth based access for drupagram, you must <a href="@url" target="_blank">register your application</a> with Instagram and add the provided keys here.', array('@url' => DRUPAGRAM_URL_APP_REGISTER)),
  );

  $form['oauth']['custom_instagram_feed_client_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Instagram Client ID'),
    '#default_value' => variable_get('custom_instagram_feed_client_id', NULL),
    '#required' => TRUE,
  );

  $form['oauth']['custom_instagram_feed_client_secret'] = array(
    '#type' => 'textfield',
    '#title' => t('Instagram Client Secret'),
    '#default_value' => variable_get('custom_instagram_feed_client_secret', NULL),
    '#required' => TRUE,
  );

  $form['oauth']['callback_url'] = array(
    '#type' => 'item',
    '#title' => t('Callback URL'),
    '#markup' => CUSTOM_INSTAGRAM_CALLBACK,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Generate and save access token'),
  );

  return $form;
}

function custom_instagram_feed_admin_form_submit($form, &$form_state) {
  module_load_include('lib.php', 'oauth_common');
  module_load_include('lib.php', 'drupagram');

  $client_id = $form_state['values']['custom_instagram_feed_client_id'];
  $client_secret = $form_state['values']['custom_instagram_feed_client_secret'];

  variable_set('custom_instagram_feed_client_id', $client_id);
  variable_set('custom_instagram_feed_client_secret', $client_secret);

  $_SESSION['custom_instagram_feed_oauth'] = array(
    'client_id' => $client_id,
    'client_secret' => $client_secret,
  );

  $drupagram = new InstagramOAuth($client_id, $client_secret);

  $callback = CUSTOM_INSTAGRAM_CALLBACK;
  $callback_uri = $drupagram->get_authorize_url($callback);

  $_SESSION['custom_instagram_feed_oauth']['instagramOAuth'] = $drupagram;

  drupal_goto($callback_uri);
}

function custom_instagram_feed_oauth_callback() {
  $code = isset($_GET['code']) ? $_GET['code'] : '';

  $client_id = isset($_SESSION['custom_instagram_feed_oauth']) ?
      $_SESSION['custom_instagram_feed_oauth']['client_id'] : '';

  $client_secret = isset($_SESSION['custom_instagram_feed_oauth']) ?
      $_SESSION['custom_instagram_feed_oauth']['client_secret'] : '';

  if ($code && $client_id && $client_secret) {
    $drupagram = $_SESSION['custom_instagram_feed_oauth']['instagramOAuth'];
    $response = $drupagram->get_access_token($code, CUSTOM_INSTAGRAM_CALLBACK);
    if ($response && isset($response['access_token']) && !empty($response['access_token'])) {
      try {
        db_delete('custom_instagram_feed_oauth')
          ->condition('oauth_client_id', $client_id)
          ->execute();
        db_insert('custom_instagram_feed_oauth')
          ->fields(array(
            'oauth_token' => $response['access_token'],
            'oauth_token_secret' => $client_secret,
            'oauth_client_id' => $client_id))
          ->execute();

        drupal_set_message(t('Successfull generate  access code for client_id: @client_id and secret code: @client_secret', array('@client_id' => $client_id, '@client_secret' => $client_secret)));
      }
      catch (Exception $e) {
        custom_instagram_feed_watchdog_custom('Custom instagram Feed', $e->getMessage(), NULL, WATCHDOG_ERROR);

        unset($_SESSION['custom_instagram_feed_oauth']);

        drupal_set_message($e->getMessage(), 'error');
        drupal_set_message(t('Incorect data'));
      }
    }
    else {
      unset($_SESSION['custom_instagram_feed_oauth']);

      custom_instagram_feed_watchdog_custom('Custom Instagram Feed', 'Incorect data. Please synchronize your input with your Instagram account', NULL, WATCHDOG_ERROR);
      drupal_set_message(t('Incorect data. Please synchronize your input with your Instagram account'));
    }
  }
  else {
    custom_instagram_feed_watchdog_custom('Custom Instagram Feed', 'Incorect data.Please synchronize your input with your Instagram account', NULL, WATCHDOG_ERROR);

    unset($_SESSION['custom_instagram_feed_oauth']);

    drupal_set_message(t('Incorect data. Please synchronize your input with your Instagram account'));
  }

  drupal_goto('admin/config/services/instagram-parser');
}
